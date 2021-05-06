#include <GPSTracker.h>

void GPSTracker::queueInsert(const char * CMTI){
    wdt_reset();

    DEBUG_PRINTLN("QUEUE INSERT")

    // Checking if queue is full
    if (_queueCommandNum == TRACKER_QUEUE_SIZE) return;

    // Checking if CMTI isnt larger than queue buffer
    if (strlen(CMTI) > TRACKER_BUFFER_SHORT) return;

    // Copy CMTI to queue
    strcpy(_commandQueue[_queueTail], CMTI);

    _queueCommandNum++;

    if (_queueTail >= TRACKER_QUEUE_SIZE - 1){
        _queueTail = 0;
    } else {
        _queueTail++;
    }

    DEBUG_PRINTLN("Queue items:")

    for (int i = 0; i < TRACKER_QUEUE_SIZE; i++){
        DEBUG_PRINT(i);
        DEBUG_PRINT(": ");
        DEBUG_PRINTLN(_commandQueue[i]);
    }
}

bool GPSTracker::queueExtract(char * CMTI, uint8_t CMTIBufferSize){
    wdt_reset();

    DEBUG_PRINTLN("QUEUE EXTRACT")

    // Check if queue is empty
    if (_queueCommandNum == 0) return false;

    // Check if command will fit into buffer
    if (strlen(_commandQueue[_queueHead]) > CMTIBufferSize) return false;

    // Clear CMTI buffer
    memset(CMTI, 0, CMTIBufferSize);

    // Copy command from queue
    strcpy(CMTI, _commandQueue[_queueHead]);

    // Clear position in queue
    memset(_commandQueue[_queueHead], 0, TRACKER_BUFFER_SHORT);

    _queueCommandNum--;

    if (_queueHead >= TRACKER_QUEUE_SIZE - 1){
        _queueHead = 0;
    } else {
        _queueHead++;
    }

    DEBUG_PRINTLN("Queue items:")
    for (int i = 0; i < TRACKER_QUEUE_SIZE; i++){
        DEBUG_PRINT(i);
        DEBUG_PRINT(": ");
        DEBUG_PRINTLN(_commandQueue[i]);
    }

    return true;
}