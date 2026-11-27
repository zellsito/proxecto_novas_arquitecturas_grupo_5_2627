#include "time_sync.h"

#include <time.h>

void initTimeSync() {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
}

String getIsoTimestamp() {
  struct tm timeinfo;
  if (getLocalTime(&timeinfo, 10)) {
    char buf[25];
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &timeinfo);
    return String(buf);
  }
  char buf[32];
  snprintf(buf, sizeof(buf), "1970-01-01T00:%02lu:%02luZ",
           (millis() / 60000UL) % 60UL, (millis() / 1000UL) % 60UL);
  return String(buf);
}
