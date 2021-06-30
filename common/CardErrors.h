#ifndef CardErrors_h
#define CardErrors_h

//#define E_SRC_SELECT_MF					0xC0020001
//#define E_SRC_PREPARE_SIG           0xC0020003
//#define E_SRC_SIGNATURE             0xC0020009
#define E_SRC_NO_CARD                     0xC002000A
#define E_SRC_NO_READERS_FOUND            0xC002000B
#define E_SRC_TYPE_NOTSUPPORTED           0xC002000C
#define E_SRC_SIGNATURE_FAILED            0xC002000D
#define E_SRC_NO_CONTEXT                  0xC002000E
#define E_SRC_TRANSMIT                    0xC002000F
#define E_SRC_FILE_NOT_FOUND              0xC0020010
#define E_SRC_START_TRANSACTION           0xC0020011
#define E_SRC_COMMAND_NOT_ALLOWED         0xC0020012
#define E_SRC_FILETYPE_NOT_SUPPORTED      0xC0020013
#define E_SRC_OUTPUT_BUFFER_TOO_SMALL     0xC0020014
#define E_SRC_CERT_NOT_FOUND              0xC0020015

#define E_PIN_LENGTH                   -10
#define E_PIN_TOO_SHORT                -11
#define E_PIN_TOO_LONG                 -12
#define E_PIN_BLOCKED                  -13
#define E_PIN_CANCELLED                -14
//#define E_SELECT_PIN_TYPE              -15
#define E_PIN_INCORRECT                -20
#define E_PIN_1_ATTEMPT                -21
#define E_PIN_2_ATTEMPTS               -22
#define E_PIN_3_ATTEMPTS               -23
#define E_PIN_TIMEOUT                  -24

#endif /* CardErrors_h */
