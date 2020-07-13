

typedef struct {
	uint32_t cs;                        /*!< Code and Status*/
	uint32_t msgId;                     /*!< Message Buffer ID*/
	uint8_t data[64];                   /*!< Data bytes of the FlexCAN message*/
	uint8_t dataLen;                    /*!< Length of data in bytes */
} flexcan_msgbuff_t;
