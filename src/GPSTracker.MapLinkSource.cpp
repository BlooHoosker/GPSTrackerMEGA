#include <GPSTracker.h>
#include <EEPROM.h>

void GPSTracker::setMapLinkSrc(const uint8_t linkSel){   
    EEPROM.update(LINK_SRC_ADDR, linkSel);
}

uint8_t GPSTracker::getMapLinkSrc(){
    return EEPROM[LINK_SRC_ADDR];
}