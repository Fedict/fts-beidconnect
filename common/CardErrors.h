#pragma once

#ifndef CardErrors_h
#define CardErrors_h

#define E_SRC_NO_CARD                     0xC002000A
#define E_SRC_NO_READERS_FOUND            0xC002000B

#define E_PIN_LENGTH                   -10
//#define E_PIN_TOO_SHORT                -11
//#define E_PIN_TOO_LONG                 -12
#define E_PIN_BLOCKED                  -13
#define E_PIN_CANCELLED                -14
//#define E_SELECT_PIN_TYPE              -15
#define E_PIN_INCORRECT                -20
#define E_PIN_1_ATTEMPT                -21
#define E_PIN_2_ATTEMPTS               -22
#define E_PIN_3_ATTEMPTS               -23
#define E_PIN_TIMEOUT                  -24

#endif /* CardErrors_h */
