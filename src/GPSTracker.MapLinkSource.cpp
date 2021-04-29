#include <GPSTracker.h>
#include <EEPROM.h>

void GPSTracker::setMapLinkSrc(const uint8_t linkSel){   
    // Saving link selection to EEPROM
    EEPROM.update(LINK_SRC_ADDR, linkSel);
    _mapLinkSrc = linkSel;
}

void GPSTracker::getMapLinkSrc(){
    // Reading link selection from EEPROM
    _mapLinkSrc = EEPROM[LINK_SRC_ADDR];
}