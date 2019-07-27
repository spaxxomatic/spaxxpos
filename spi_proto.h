
#define FOREACH_ELEM(ERRCODE) \
        ERRCODE(ACK_OK)   \
        ERRCODE(ACK_NOK)  \
        ERRCODE(ACK_UNKNOWN)   \
        ERRCODE(ACK_ERR_INVALID_STARTBYTE)  \
        ERRCODE(ACK_CONN_OK)  \
        ERRCODE(ACK_RESET)  \
        ERRCODE(ACK_ERR_INVALID_COMMAND) \
        ERRCODE(ACK_ERR_TIMEOUT_WAITING_SERVO_RESPONSE) \
        ERRCODE(ACK_ERR_BUFFER_OVERFLOW) \
        ERRCODE(ERR_QS_COMM_TIMEOUT) \

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

enum SPI_QS_ERRORCODES {
    FOREACH_ELEM(GENERATE_ENUM)
    LASTELEM = ERR_QS_COMM_TIMEOUT
};

static const char *SPI_QS_ERRORCODES_STRINGS[] = {
    FOREACH_ELEM(GENERATE_STRING)
};

static const char* getSpiCommMsgText(uint8_t errorCode){
    if (errorCode <= LASTELEM)
        return SPI_QS_ERRORCODES_STRINGS[errorCode];
    else  return "SPI_ERR_UNKNOWN" ;
}

