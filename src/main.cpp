// Copyright (c) 2023 Inaba (@hollyhockberry)
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php

#include <M5Unified.h>
#include <ChatClient.h>

namespace {
constexpr const char* OPENAI_ROOT_CA = \
  "-----BEGIN CERTIFICATE-----\n" \
  "MIIDdzCCAl+gAwIBAgIEAgAAuTANBgkqhkiG9w0BAQUFADBaMQswCQYDVQQGEwJJ\n" \
  "RTESMBAGA1UEChMJQmFsdGltb3JlMRMwEQYDVQQLEwpDeWJlclRydXN0MSIwIAYD\n" \
  "VQQDExlCYWx0aW1vcmUgQ3liZXJUcnVzdCBSb290MB4XDTAwMDUxMjE4NDYwMFoX\n" \
  "DTI1MDUxMjIzNTkwMFowWjELMAkGA1UEBhMCSUUxEjAQBgNVBAoTCUJhbHRpbW9y\n" \
  "ZTETMBEGA1UECxMKQ3liZXJUcnVzdDEiMCAGA1UEAxMZQmFsdGltb3JlIEN5YmVy\n" \
  "VHJ1c3QgUm9vdDCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAKMEuyKr\n" \
  "mD1X6CZymrV51Cni4eiVgLGw41uOKymaZN+hXe2wCQVt2yguzmKiYv60iNoS6zjr\n" \
  "IZ3AQSsBUnuId9Mcj8e6uYi1agnnc+gRQKfRzMpijS3ljwumUNKoUMMo6vWrJYeK\n" \
  "mpYcqWe4PwzV9/lSEy/CG9VwcPCPwBLKBsua4dnKM3p31vjsufFoREJIE9LAwqSu\n" \
  "XmD+tqYF/LTdB1kC1FkYmGP1pWPgkAx9XbIGevOF6uvUA65ehD5f/xXtabz5OTZy\n" \
  "dc93Uk3zyZAsuT3lySNTPx8kmCFcB5kpvcY67Oduhjprl3RjM71oGDHweI12v/ye\n" \
  "jl0qhqdNkNwnGjkCAwEAAaNFMEMwHQYDVR0OBBYEFOWdWTCCR1jMrPoIVDaGezq1\n" \
  "BE3wMBIGA1UdEwEB/wQIMAYBAf8CAQMwDgYDVR0PAQH/BAQDAgEGMA0GCSqGSIb3\n" \
  "DQEBBQUAA4IBAQCFDF2O5G9RaEIFoN27TyclhAO992T9Ldcw46QQF+vaKSm2eT92\n" \
  "9hkTI7gQCvlYpNRhcL0EYWoSihfVCr3FvDB81ukMJY2GQE/szKN+OMY3EU/t3Wgx\n" \
  "jkzSswF07r51XgdIGn9w/xZchMB5hbgF/X++ZRGjD8ACtPhSNzkE1akxehi/oCr0\n" \
  "Epn3o0WC4zxe9Z2etciefC7IpJ5OCBRLbf1wbWsaY71k5h+3zvDyny67G7fyUIhz\n" \
  "ksLi4xaNmjICq44Y3ekQEe5+NauQrz4wlHrQMz2nZQ/1/I6eYs9HRCwBXbsdtTLS\n" \
  "R9I4LtD+gdwyah617jzV/OeBHRnDJELqYzmp\n" \
  "-----END CERTIFICATE-----\n";

ChatClient _client(OPENAI_API_KEY, OPENAI_ROOT_CA);
String _message = "";

}  // namespace

void setup() {
  M5.begin();

  if (WIFI_SSID == "" || WIFI_PSK == "") {
    WiFi.begin();
  } else {
    WiFi.begin(WIFI_SSID, WIFI_PSK);
  }
  while (WiFi.status() != WL_CONNECTED) {
    ::delay(1000);
    M5_LOGI("Connecting to WiFi...");
  }
  M5_LOGI("Connected to WiFi");

  _client.begin();
  _client.AddSystem(
    "感情を表現した会話のシミュレーションを行います。"\
    "会話におけるあなたの感情を[]で囲んで回答の先頭に付加してください。"\
    "感情は Happy, Angry, Sad, Doubt, Sleepy のいずれかを選択してください。"
  );
  _message = "";
}

bool split(const String& str, String& message, String& emotion) {
  message = "";
  emotion = "";
  auto n = 0;
  while (true) {
    auto s = str.indexOf('[', n);
    auto e = str.indexOf(']', n);
    if (s < 0 || e < 0) {
      message += str.substring(n);
      break;
    }
    if (s > 0) {
      message += str.substring(n, s);
    }
    emotion = str.substring(s + 1, e).c_str();
    n = e + 1;
  }
  return true;
}

void loop() {
  const auto ch = Serial.read();
  String res;
  if (ch >= 0) {
    switch (ch) {
    case '\n':
    case '\r':
      if (!_message.isEmpty()) {
        Serial.printf("You: %s\r\n", _message.c_str());
        if (_client.Chat(_message.c_str(), res)) {
          String message, emotion;
          if (split(res, message, emotion)) {
            Serial.printf("GPT: %s (emotion=%s)\r\n", message.c_str(), emotion.c_str());
          }
        }
        _message.clear();
      }
      break;
    default:
      _message += (char)ch;
      break;
    }
  }
}
