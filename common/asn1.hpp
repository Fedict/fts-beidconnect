#ifndef __ASN1_H__
#define __ASN1_H__

#ifdef __cplusplus
extern "C" {
#endif

/* return codes for ASN1 parsing */
#define E_ASN_BAD_TAG           -1           /* tag not expected or unknown */
#define E_ASN_TAG_LEN           -2           /* tag has length > 4 bytes */
#define E_ASN_BAD_LEN           -3           /* length encoding error */
#define E_ASN_INCOMPLETE        -4           /* unexpected end of ASN1 encoding */
#define E_ASN_ITEM_NOT_FOUND    -5           /* item in path does not exist */
#define E_ASN_ALLOC             -6           /* memory allocation error */
#define E_ASN_LIST              -7           /* error in asn1 item list to encode */
#define E_ASN_OUTBUF_TOO_SMALL  -8
#define E_ASN_EXTENDED_TAG      -10          /* extended tags not supported for encoding */
#define E_ASN_INVALID           -11          /* bad asn1 encoding */
//#define ASN1_ERR -1


typedef struct ASN1_ITEM 
{
	unsigned int   tag;
	unsigned char  *p_data;
	unsigned int	l_data;
   unsigned int   nsubitems;
   unsigned char  *p_raw;
   unsigned int   l_raw;
} ASN1_ITEM;

typedef struct ASN1_LIST
{
   unsigned int   size;
   unsigned int   nitems;
   ASN1_ITEM      *item;
} ASN1_LIST;



typedef struct T_OID
{
	const char *dotid;
	const char *str;
} T_OID;



/* BER ASN1 decoding masks */
#define TAG_MASK   0x1F  /* bits 1 - 5  */
#define TYPE_MASK  0x20  /* bit  6      */
#define CLASS_MASK 0xC0  /* bits 7,8    */

#define LEN_MASK   0x7F  /* bits 1 - 7  */
#define EXT_LEN    0x80  /* bit  8      */

/* For this implementation: class, type and tagnumber are converted to single unsigned int max 4 bytes (32-bit) */
/* 2 bits for class, 1 bit for type so 29 bits remain for tagnumber (32-bit system)    */
/* tag encoded as multiple of 7 bits cannot be bigger than 28 bits anyway */
/* following are the definitions for class, type and value */
/* ASN_CLASS: bit 1 and 2 */
#define ASN_UNIVERSAL    0x00
#define ASN_APPLICATION  0x01
#define ASN_CONTEXT      0x02
#define ASN_PRIVATE      0x03

/* ASN_TYPE: bit 3 */
#define ASN_PRIMITIV     0x00
#define ASN_CONSTRUCTED  0x04

/* COMMON ASN_TAGS */
//#define ASN_EOC               ( ASN_CONTEXT   | ASN_CONSTRUCTED | (0x00 << 3) )   /* end of contents octet */
#define ASN_BOOLEAN             ( ASN_UNIVERSAL | ASN_PRIMITIV    | (0x01 << 3) )      
#define ASN_INTEGER             ( ASN_UNIVERSAL | ASN_PRIMITIV    | (0x02 << 3) )      
#define ASN_BIT_STRING          ( ASN_UNIVERSAL | ASN_PRIMITIV    | (0x03 << 3) )
#define ASN_OCTET_STRING        ( ASN_UNIVERSAL | ASN_PRIMITIV    | (0x04 << 3) )
#define ASN_NULL                ( ASN_UNIVERSAL | ASN_PRIMITIV    | (0x05 << 3) )    
#define ASN_OID                 ( ASN_UNIVERSAL | ASN_PRIMITIV    | (0x06 << 3) )    
#define ASN_ENUMERATED          ( ASN_UNIVERSAL | ASN_PRIMITIV    | (0x0A << 3) )
#define ASN_UTF8_STRING         ( ASN_UNIVERSAL | ASN_PRIMITIV    | (0x0C << 3) )
#define ASN_SEQUENCE            ( ASN_UNIVERSAL | ASN_CONSTRUCTED | (0x10 << 3) )  
#define ASN_SET                 ( ASN_UNIVERSAL | ASN_CONSTRUCTED | (0x11 << 3) )
#define ASN_PRINTABLE_STRING    ( ASN_UNIVERSAL | ASN_PRIMITIV    | (0x13 << 3) )
#define ASN_IA5STRING           ( ASN_UNIVERSAL | ASN_PRIMITIV    | (0x16 << 3) )
#define ASN_UTCTIME             ( ASN_UNIVERSAL | ASN_PRIMITIV    | (0x17 << 3) )
#define ASN_GENERALIZEDTIME     ( ASN_UNIVERSAL | ASN_PRIMITIV    | (0x18 << 3) )
#define ASN_ANY                 ( ASN_UNIVERSAL | ASN_PRIMITIV    | (0xfffff << 3) )

/* ASN1 structured element */
#define ASN_SE(a)               ( ASN_CONTEXT   | ASN_CONSTRUCTED | (a << 3) )

/* ASN1 tagged element */
#define ASN_TE(a)               ( ASN_CONTEXT   | ASN_PRIMITIV    | (a << 3) )

#define ASN_FALSE               0x00
#define ASN_TRUE                0xFF


#define OID_RSA_ENCRYPTION      "\x2A\x86\x48\x86\xF7\x0D\x01\x01\x01"

/* ASN1 decoding functions */
int asn1_get_item(const unsigned char *content, unsigned int len, const char *path, ASN1_ITEM *item);
int asn1_find_item(const unsigned char *content, unsigned int len, unsigned int findtag, ASN1_ITEM *item);
int asn1_find_item_simple(const unsigned char *content, unsigned int len, unsigned int findtag, ASN1_ITEM *item);
int asn1_dec_oid( unsigned char *p_data, int l_data, unsigned int *oid, int *l_oid);
char* oid2str(unsigned char* p_data, int l_data);
   
/* ASN1 encoding functions */
/* !!!! asn1_add_item adds the pointer to a list of elements to encode, but does not copy the data */
int asn1_add_item(ASN1_LIST *list, unsigned int tag, unsigned char *p_data, unsigned int l_data, unsigned int nsubitems);
int get_item_length(ASN1_LIST *list, unsigned int n, unsigned int *l, unsigned int *nsubitems);
int asn1_encode_list(ASN1_LIST *list, unsigned char *buf, unsigned int *l_buf);
void asn_clear_list(ASN1_LIST *list);

/* Helper functions: decode - encode bitstrings
 * LSB of the unsigned int corresponds to most left bit in the bitstring. 
 * Unused bits are added in front of bitstring
 * Make sure that output buffer for encoding is always sizeof(unsigned int)+1 !!! 
 * even if bits can be encoded in smaller sizes, l_out will output real size
 * independent of platform, endian and integer length */
unsigned int asn_bitstring2ui(unsigned char *in, unsigned int l_in);
void asn_ui2bitstring(unsigned int in, unsigned char *out, unsigned int *l_out);
int asn_compare_items(ASN1_ITEM* item1, ASN1_ITEM* item2);
   
int dump_asn1(unsigned char* in, unsigned int l_in, int level, char** dump);
int get_printable_dn(ASN1_ITEM *p_dn, char **pp_dn, size_t *p_l_dn);
   
/* Helper functions: encode uint as ASN_INTEGER */
void encode_uint(unsigned int in, unsigned char *out, unsigned int *l_out);

#ifdef __cplusplus
   }
#endif

#endif
