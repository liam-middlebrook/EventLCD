/* Display controller. */

#include "display.h"

Display::Display(
    uint8_t data, uint8_t clock, uint8_t latch, uint8_t width,
    uint8_t height) {
  this->width = width;
  this->height = height;

  lcd = new LiquidCrystal595(data, latch, clock);
  lcd->begin(width, height);
  lcd->clear();

  network_status = DISCONNECTED;
  request = 0;
  content = 0;

  needs_update = true;
}

void Display::update(float elapsed) {
  switch (screen) {
    case SCREEN_NETWORK:
      if (request) {
        RequestState new_state = request->getState();
        if (new_state != prev_request_state) {
          needs_update = true;
          prev_request_state = new_state;
        }
      }

      if (needs_update) {
        lcd->clear();
        lcd->setCursor(0, 0);
        lcd->print("Status: ");
        if (network_status == DISCONNECTED)
          lcd->print("disconnected");
        else if (network_status == CONNECTED)
          lcd->print("connected");
        else if (network_status == CONNECTING)
          lcd->print("connecting");
        else if (network_status == RECONNECTING)
          lcd->print("reconnecting");
        else if (network_status == DHCP_FAILED)
          lcd->print("DHCP failed");

        // Print MAC address
        lcd->setCursor(0, 1);
        for (int i = 0; i < 6; i++) {
          if (mac[i] < 0x10) lcd->print(0);
          lcd->print(mac[i], HEX);
          if (i < 5) lcd->print(":");
        }

        if (network_status == CONNECTED) {
          // Print IP address
          lcd->setCursor(0, 2);
          for (int i = 0; i < 4; i++) {
            lcd->print(((byte*)&ip)[i], DEC);
            if (i < 3) lcd->print(".");
          }

          // Print HTTP request information
          if (request) {
            lcd->setCursor(0, 3);

            if (request->failed()) {
              printFirst(request->getErrorMessage(), 20);
            } else {
              lcd->print("HTTP: ");
              printFirst(request->getStatusString(), 20 - 6);
            }
          }
        }
      }
      break;

    case SCREEN_TEXT:
      if (needs_update) {
        lcd->clear();
        char *c = content;

        // For each row in the display
        for (uint8_t r = 0; r < height && *c; r++) {
          lcd->setCursor(0, r);

          // Print each character until null or newline is reached, or if the
          // end of the row is reached.
          for (uint8_t col = 0; col < width && *c && *c != '\n'; col++, c++)
            lcd->print(*c);

          // If current character is newline, skip it!
          if (*c == '\n') c++;
        }
      }
      break;
  }

  needs_update = false;
}

void Display::printFirst(const char *str, uint16_t length) {
  for (int i = 0; i < length && str[i]; i++)
    lcd->print(str[i]);
}

Screen Display::getScreen() {
  return screen;
}

void Display::setScreen(Screen screen) {
  this->screen = screen;
  needs_update = true;
}

void Display::setMAC(uint8_t mac[]) {
  for (int i = 0; i < 6; i++)
    this->mac[i] = mac[i];
  if (screen == SCREEN_NETWORK)
    needs_update = true;
}

void Display::setIP(uint32_t ip) {
  this->ip = ip;
  if (screen == SCREEN_NETWORK)
    needs_update = true;
}

void Display::setNetworkStatus(NetworkStatus status) {
  network_status = status;
  if (screen == SCREEN_NETWORK)
    needs_update = true;
}

void Display::setRequest(Request *request) {
  this->request = request;
  if (screen == SCREEN_NETWORK)
    needs_update = true;
}

void Display::setContent(const char *content) {
  this->content = (char*)content;
  if (screen == SCREEN_TEXT)
    needs_update = true;
}

