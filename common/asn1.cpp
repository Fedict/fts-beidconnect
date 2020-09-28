#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include "asn1.hpp"
#include "log.hpp"
#include "util.h"
#include <stdarg.h>


char* oid2str(unsigned char* p_data, int l_data)
{
   static char oidstr[128] = "";
   
   unsigned int oid[20];
   int oidLen = 20;
   //memset(oidstr, 0, 128);
   asn1_dec_oid(p_data, l_data, oid, &oidLen);
   
   for (int i = 0; i<oidLen && strlen(oidstr)<110; i++) {
      if (i==0)
         sprintf(oidstr, "%d", oid[i]);
      else
         sprintf(oidstr, "%s.%d", oidstr, oid[i]);
   }
//   strcat(oidstr, "\"");
   return oidstr;
}

char* bitstring2str(unsigned char* p_data, int l_data)
{
   static char str[33];
   unsigned int value = 0;
   memset(str, 0, sizeof(str));
   value = asn_bitstring2ui(p_data, l_data);
   
   int i = 31;
   int isFirstBitSet = 0;
   while (i--) {
      int b = value & (1<<i);
      if (b)
         isFirstBitSet = 1;
      if (isFirstBitSet)
         strcat(str, (b)?"1":"0" );
   }
   return str;
}

/* e.g. path = "\1\1\2" */
/* skips a number of items */
int skip_item(unsigned char *in, unsigned int l_in, unsigned int n, unsigned char **out, unsigned int *l_out)
{
//int ret = 0;
unsigned int  classtag, type, tag, l = 0;
unsigned char *p = in;
unsigned char *end = in + l_in - 1;  //points to last byte from input stream
unsigned int i,j;
unsigned int l_tag;
unsigned int l_len;
int indefinite = 0;

for (i=1; i < n; i++)
	{
	classtag = *p & CLASS_MASK;
	type     = *p & TYPE_MASK;
	tag      = *p & TAG_MASK;

	if (tag == TAG_MASK)
		{
		/* tag = sequence of 7 bit values */
		tag = 0;
		l_tag = 0;
		do 
			{
			p++;
			l_tag++;
			if (l_tag > 4)
            /* tag has length > 4 times 7 bit */
				return (E_ASN_TAG_LEN);
			tag = (tag << 7) | (*p & LEN_MASK);
			} while ((*p & EXT_LEN) && (p < end));
		}

	if (p > end) //???check this: if p==end and length = 0, this could be the case for a NULL element, otherwise p should be < end
		return (E_ASN_INCOMPLETE);
	
	/* get length of asn1_item */
   p++;
	l = l_len = (unsigned int) *p;
	if (l_len & EXT_LEN)
		{
		if ((l_len &= LEN_MASK) > 4)
			return (E_ASN_BAD_LEN);
		if (l_len == 0)
			/* indefinite length */
         indefinite = 1;
      l = 0;
      for (j=0; j < l_len; j++)
			{
         p++;
			if (p > end)
				return (E_ASN_INCOMPLETE);
			l = (l << 8) | *p;
			}
		}

	/* skip tag, length and data to get next item */
	p = p + l + 1;
   if (p > end)
      return (E_ASN_ITEM_NOT_FOUND);
	}

*out = p;
*l_out = l_in - (unsigned int)(*out-in);

return (0);
}


int asn1_get_item(const unsigned char *content, unsigned int len, const char *path, ASN1_ITEM *item)
{
unsigned int  classtag = 0;
unsigned int type = 0;
unsigned int tag = 0;
unsigned char *p = (unsigned char*) content;
unsigned int l = len;
unsigned int l_data;
unsigned char *end = (unsigned char*)content + len - 1;
unsigned char *P = (unsigned char*) path;
int indefinite = 0;
int ret = 0;

memset(item, 0, sizeof (ASN1_ITEM));

for (; *P; P++)
	{
	ret = skip_item(p, l, *P, &p, &l);
	if (ret)
		return (ret);
	
   /* check if we are decoding inside a BIT STRING: tag == parent_tag */
   /* first octet of bit string is the number od unused bits at the end of the bitstring */
   /* in CER/DER: unused bits are always zero */
   /* !!! if we are to look inside BITSTRING and unencoded bits are 0, then we skip the uncoded bits and move to next byte inside bitstring */
   if ((tag == 0x03) && (*p == 0) && (*(P+1) != 0) )
      {
      // in case if bit string, we just omit the unused bits if they are zero
      p++;
      l--;
      }

   classtag = *p & CLASS_MASK;
   type     = *p & TYPE_MASK;
   tag      = *p & TAG_MASK;

   if (tag == TAG_MASK)
	   {
	   /* tag = sequence of 7 bit values */
	   tag = 0;
	   do 
		   {
         p++;
		   if (p > content + 4)
			   return (E_ASN_TAG_LEN);
		   tag = (tag << 7) | (*p & LEN_MASK);
		   } while ((*p & EXT_LEN) && (p < end));
	   }			

   if (p == end) //????
	   return (E_ASN_INCOMPLETE);

   p++;
   l = l_data = (unsigned int) *p;
   if (l_data & EXT_LEN)
	   {
	   if ((l_data &= LEN_MASK) > 4)
		   return (E_ASN_BAD_LEN);
	   if (l_data == 0)
		   indefinite = 1;
      l = 0;
	   while (l_data--)
		   {
         p++;
		   if (p > end)
			   return (E_ASN_INCOMPLETE);
		   l = (l << 8) | *p;
		   }
	   }
   p++;
   }

/* p_data points to element itself after length encoding, not the tag */
item->p_data = p;
item->l_data = l;
/* construct tag: bit 1 and 2 for class, bit 3 for primitiv or structured and tagnumùber from bit 4-32 = 29 bits for number encoding */
item->tag = (classtag >> 6) | (type >> 3 ) | (tag << 3);

return (0);
}


//preliminary version to find items with particular TAG in linear list of ASN1 items
int asn1_find_item(const unsigned char *content, unsigned int len, unsigned int findtag, ASN1_ITEM *item)
{
unsigned int  classtag, type, tagnum, tag = 0;
unsigned char *p = (unsigned char*) content;
unsigned int l = len;
unsigned int l_data;
unsigned char *end = (unsigned char*)content + len - 1;
int indefinite = 0;
int found = 0;
//int ret = 0;

memset(item, 0, sizeof (ASN1_ITEM));

do
   {
   if ((p == 0) || (len == 0))
      return (E_ASN_ITEM_NOT_FOUND);
      
   classtag = *p & CLASS_MASK;
   type     = *p & TYPE_MASK;
   tagnum   = *p & TAG_MASK;

   if (tagnum == TAG_MASK)
      {
      /* tagnum = sequence of 7 bit values */
      tagnum = 0;
      do 
	      {
         p++;
	      if (p > content + 4)
		      return (E_ASN_TAG_LEN);
	      tagnum = (tagnum << 7) | (*p & LEN_MASK);
	      } while ((*p & EXT_LEN) && (p < end));
      }			

   tag = (classtag >> 6) | (type >> 3 ) | (tagnum << 3);

   if (p == end) //????
      return (E_ASN_INCOMPLETE);

   p++;
   l = l_data = (unsigned int) *p;
   if (l_data & EXT_LEN)
      {
      if ((l_data &= LEN_MASK) > 4)
	      return (E_ASN_BAD_LEN);
      if (l_data == 0)
	      indefinite = 1;
      l = 0;
      while (l_data--)
	      {
         p++;
	      if (p > end)
		      return (E_ASN_INCOMPLETE);
	      l = (l << 8) | *p;
	      }
      }

   if (findtag == tag)
      {
      //point to value
      p++;
      found = 1;
      }
   else
      //point to next item
      p = p + l + 1;

   if (p > end)
      return (E_ASN_ITEM_NOT_FOUND);
   } while (found == 0);

if (!found)
   return (E_ASN_ITEM_NOT_FOUND);

item->p_data = p;
item->l_data = l;
item->tag = findtag;

return (0);
}


int asn1_find_item_simple(const unsigned char *content, unsigned int len, unsigned int findtag, ASN1_ITEM *item)
{
   unsigned int  tag = 0;
   unsigned char *p = (unsigned char*) content;
   unsigned int l = len;
   unsigned int l_data;
   unsigned char *end = (unsigned char*)content + len - 1;
   int indefinite = 0;
   int found = 0;
   
   memset(item, 0, sizeof (ASN1_ITEM));
   
   do
   {
      if ((p == 0) || (len == 0))
         return (E_ASN_ITEM_NOT_FOUND);
      
      tag = *p;
      
      if (p == end) //????
         return (E_ASN_INCOMPLETE);
      
      p++;
      l = l_data = (unsigned int) *p;
      if (l_data & EXT_LEN)
      {
         if ((l_data &= LEN_MASK) > 4)
            return (E_ASN_BAD_LEN);
         if (l_data == 0)
            indefinite = 1;
         l = 0;
         while (l_data--)
         {
            p++;
            if (p > end)
               return (E_ASN_INCOMPLETE);
            l = (l << 8) | *p;
         }
      }
      
      if (findtag == tag)
      {
         //point to value
         p++;
         found = 1;
      }
      else
         //point to next item
         p = p + l + 1;
      
      if (p > end)
         return (E_ASN_ITEM_NOT_FOUND);
   } while (found == 0);
   
   if (!found)
      return (E_ASN_ITEM_NOT_FOUND);
   
   item->p_data = p;
   item->l_data = l;
   item->tag = findtag;
   
   return (0);
}



/* decode asn1 oid to array of numbers */
int asn1_dec_oid( unsigned char *p_data, 
             int           l_data, 
             unsigned int  *oid, 
             int           *l_oid)
{
int           len = 0;
unsigned int  id = 0;
int maxOidLen = *l_oid;
*l_oid = 0;

while(l_data > 0 && *l_oid < maxOidLen)
  {
  id = (id << 7) + (*p_data & 0x7f);
  if((*p_data & 0x80) == 0)
    {
    if(len == 0)
      {
      *oid++ = id / 40;
      id %= 40;
      (*l_oid)++;
      len++;
      }
    *oid++ = id;
    (*l_oid)++;
    len++;
    id = 0;
    }
  p_data++;
  l_data--;
  }

return(len);
}






unsigned int asn_bitstring2ui(unsigned char *in, unsigned int l_in)
{
unsigned int bits = 0;
unsigned char *p = in+1;
int i, j;

if (l_in == 1)
  //only unused bits present, all bits zero
  return(0);

//put first bit (most left bit of first asn1 byte) in LSB of flags (unsinged int)
//so asn1 bytes are swapped before keeping in unsigned int
for (i=0; i < (int)(l_in-1); p++, i++)
  {
  for (j=7; j>=0; j--)
    {
    bits |= ((*p >> j) & 0x01) << ((7-j) + (i*8));
    }
  }
return(bits);
}


void asn_decode_dn(unsigned char* in, unsigned int l_in, char* dn_keys[], char* dn_values[])
{
   
}

int asn_compare_dn(char* dn1[][2], char* dn2[][2])
{
#define DN_KEY_OID  0
#define DN_VALUE    1
   
   for (int i = 0; ;i++) {

      if ((dn1[i][DN_KEY_OID] == 0) ^ (dn2[i][DN_KEY_OID] == 0))
         return -1; //diff in dn components
      if ((dn1[i][DN_KEY_OID] == 0) && (dn2[i][DN_KEY_OID] == 0))
         return 0;  //end of both dn's
      
      for (int j=0; ; j++) {
         
         if (dn2[j][DN_KEY_OID] == 0)
             return -1; //reached end of dn2 without oid found that we were looking for
         
         if (strcmp(dn1[i][DN_KEY_OID], dn2[j][DN_KEY_OID]) != 0)
            continue;   //this is not the key we want to compare
         if (strcmp(dn1[i][DN_VALUE], dn2[j][DN_VALUE]) != 0)
            return -1;   //value is different, dn's are not equal
         else
            break; //continue with next key component of dn
      }
   }
   return -1;
}

int asn_compare_items(ASN1_ITEM* item1, ASN1_ITEM* item2)
{
   //get all childs from item1
   //is there an child in item2 that has the same values as item1
   if (item1->l_data != item2->l_data)
      return -1;
   if (memcmp(item1->p_data, item2->p_data, item1->l_data) != 0)
      return -1;
   
   return 0;
}

#define MAX_OID_STR_SIZE 1024
#define MAX_OID_SIZE  32

#define OID_TABLE {  {"2.5.4.3", "/CN"}, \
{"2.5.4.5", "/SN"}, \
{"2.5.4.6", "/C"}, \
{"2.5.4.7", "/L"}, \
{"2.5.4.8", "/ST"}, \
{"2.5.4.10", "/O"}, \
{"2.5.4.11", "/OU"}, \
{"2.5.4.12", "/T"}, \
{"2.5.4.13", "/D"}, \
{"1.2.840.113549.1.9.1", "/Email"}, \
{"0.2.262.1.10.7.20", "/ND"}, \
{"1.2.840.113549.1.1.1", "RSA encryption"}, \
{ NULL, NULL } }

/* convert oid to readable string */
const char* oid2PrintableStr(unsigned char* oid, int l_oid)
{
   int l_str = 128;
   int i;
   T_OID *p_entry = NULL;
   T_OID p_oid_table[]=OID_TABLE;
   char* oidstr;
   
   if (l_oid == 0)
   {
      l_str = 0;
      return ("???");
   }
   
   oidstr = oid2str(oid, l_oid);
   
   /* if dotted id in table => translate to short form "/C" */
   p_entry = &p_oid_table[0];
   for (i=0; p_entry->dotid ; p_entry = &p_oid_table[++i])
   {
      if (strcmp(p_entry->dotid, oidstr) == 0)
      {
         //if (l_str < strlen(p_entry->str))
         //   return(oidstr);
         //strcpy(oidstr, p_entry->str);
         //break;
            return (p_entry->str);
      }
   }
   
   return(oidstr);
}


/* convert asn1 encoded DN to printable string according to table of DN items defined here */
int get_printable_dn(ASN1_ITEM     *p_dn,   //I: points to the DN struct
                     char          **pp_dn, //O: contains printable DN
                     unsigned int           *p_l_dn) //O: length of pp_dn
{
   int          ret;
   char path[3];           // contains the number of dn attributes p_dn can contain
   char *p_buf = NULL;
   int          l_buf = 0;
   unsigned int  ofs = 0;
   int i, n;
   ASN1_ITEM setitem;
   ASN1_ITEM item;
   char abrv[MAX_OID_STR_SIZE];
   size_t  l_abrv = sizeof(abrv);
   
   if(p_dn->tag != ASN_SEQUENCE)
      return(E_ASN_ITEM_NOT_FOUND);
   
   //look for SET elements inside the SEQUENCE
   path[0] = 1;
   path[1] = 0;
   
   for (n=1; ;n++)
   {
      path[0] = n;
      
      ret = asn1_get_item(p_dn->p_data, p_dn->l_data, path, &setitem);
      if (ret)
         break;
      
      if(setitem.tag != ASN_SET)
         return (E_ASN_BAD_TAG);
      
      for (i=1;;i++)
      {
         path[0] = i;
         path[1] = 0;
         ret = asn1_get_item(setitem.p_data, setitem.l_data, path, &item);
         if (ret)
            break;
         if(item.tag == ASN_SEQUENCE)
         {
            path[1] = 1;
            path[2] = 0;
            ret = asn1_get_item(setitem.p_data, setitem.l_data, path, &item);
            if (ret)
               break;
            else if (item.tag != ASN_OID )
               continue;
            
            strcpy(abrv, oid2PrintableStr(item.p_data, item.l_data));
            l_abrv = strlen(abrv);
            
            path[1] = 2;
            path[2] = 0;
            ret = asn1_get_item(setitem.p_data, setitem.l_data, path, &item);
            if (ret)
               continue;
            
            /* construct this: '/C=BE /CN="Vital Schonkeren" ...' */
            if(memchr(item.p_data, ' ', item.l_data) != NULL)
            {
               l_buf += l_abrv + 4 + item.l_data;
               if( (p_buf = (char*)realloc(p_buf, l_buf) ) == NULL)
                  return(-1);
               
               memcpy(p_buf + ofs, abrv, l_abrv);
               ofs += l_abrv;
               memcpy(p_buf + ofs, "=\"", 2);
               ofs += 2;
               memcpy(p_buf + ofs, item.p_data, item.l_data);
               ofs += item.l_data;
               memcpy(p_buf + ofs, "\" ", 2);
               ofs += 2;
            }
            else
            {
               l_buf += l_abrv + 2 + item.l_data;
               if( (p_buf = (char*)realloc(p_buf, l_buf) ) == NULL)
                  return(-1);
               
               memcpy(p_buf + ofs, abrv, l_abrv);
               ofs += l_abrv;
               memcpy(p_buf + ofs, "=", 1);
               ofs++;
               memcpy(p_buf + ofs, item.p_data, item.l_data);
               ofs += item.l_data;
               memcpy(p_buf + ofs, " ", 1);
               ofs++;
            }
         }
      }
   }
   
   if(l_buf && p_buf != NULL)
   {
      p_buf[l_buf-1] = 0;
   }
   
   if(pp_dn && p_l_dn)
   {
      *pp_dn = (char*)p_buf;
      *p_l_dn = l_buf - 1;  /* the last byte was reserved for a space */
   }
   else
      free(p_buf);
   
   return(0);
}

int asn1_add_item(ASN1_LIST *list, unsigned int tag, unsigned char *p_data, unsigned int l_data, unsigned int nsubitems)
{
/* allocate more mem if necessary */
if (list->size == list->nitems)
   {
   list->item = (struct ASN1_ITEM *)realloc(list->item, (list->size+10)*sizeof(ASN1_ITEM));
   if (list->item == NULL)
      return (E_ASN_ALLOC);
   list->size += 10;
   }

/* add element*/
list->item[list->nitems].tag = tag;
list->item[list->nitems].p_data = p_data;
list->item[list->nitems].l_data = l_data;
list->item[list->nitems].nsubitems = nsubitems;

list->nitems += 1;

return (0);
}


void asn_clear_list(ASN1_LIST *list)
{
if (list->item != 0)
  free(list->item);
list->item = NULL;
list->nitems = 0;
list->size = 0;
}


int asn1_encode_list(ASN1_LIST *list, unsigned char *buf, unsigned int *l_buf)
{
int ret = 0;
unsigned int  classtag, type, tagnum, tag = 0;
unsigned char *p = buf;
unsigned int l, len = 0;
unsigned char lenbytes = 0;
unsigned int nsubitems = 0;
unsigned int i;
int j; // signed int!!!
unsigned int l_max = *l_buf;

*l_buf = 0;

for (i=0; i < list->nitems; i++)
   {
   /* calculate add tag */
   classtag = (list->item[i].tag & 0x3) << 6;
   type = (list->item[i].tag & 0x4) << 3;
   tagnum = (list->item[i].tag & ~0x7) >> 3;

   if (tagnum < 0x1F) {
      tag = classtag | type | tagnum;
      *p++ = tag;
   }
   else if (tagnum == 0xFFFFF) {
      //experimental sept 2015 to encode ASN_ANY
      /* add data */
      if (*l_buf + list->item[i].l_data <= l_max) {
         memcpy(p, list->item[i].p_data, list->item[i].l_data);
         p += list->item[i].l_data;
         continue;
      }
      else {
         return(E_ASN_OUTBUF_TOO_SMALL);
      }
   }
   else {
      return E_ASN_EXTENDED_TAG;
   }

   /* calculate length and */
   ret = get_item_length(list, i, &len, &nsubitems);
   if (ret)
      return (ret);
   l=len;

   /* add length */
   if (len >= 0xFFFFFFFF)
      return (E_ASN_BAD_LEN);
   else if (len <= 0x7F)
      lenbytes = 1;       /* short form */
   else
      {
      lenbytes = 0;
      while (l)
         {
         lenbytes += 1;
         l >>= 8;
         }
      *p++ = 0x80 + lenbytes;
      }
   
   for (j=lenbytes-1; j >= 0;j--)
      {
      *p++ = (len >> (j*8)) & 0xFF;
      }

   /* add data */
   if (*l_buf + list->item[i].l_data <= l_max)
      {
      memcpy(p, list->item[i].p_data, list->item[i].l_data);
      p += list->item[i].l_data;
      }
   else
      {
      return(E_ASN_OUTBUF_TOO_SMALL);
      }
   }

*l_buf = (unsigned int) (p-buf);

return (ret);
}




int get_item_length(ASN1_LIST *list, unsigned int n, unsigned int *l, unsigned int *nsubitems)
{
int ret = 0;
unsigned int i;
ASN1_ITEM *item;
unsigned int len = 0;
unsigned int nsubs = 0;
unsigned int lenbytes = 0;
//unsigned int taglen = 0;

*nsubitems = 0;

if ( n > list->nitems)
   return (E_ASN_LIST);

item = &list->item[n];
*l = item->l_data;

//if there are childs here, we search recursively and add extra bytes for length and tag
for (i=1; i <= item->nsubitems; i++)
   {
   ret = get_item_length(list, n+i+*nsubitems, &len, &nsubs);
   if (ret)
      return (ret);
   *l += len;

   //experimental
   //for ASN_ANY no tag nor lengthbytes are added
      if (list->item[n+i+*nsubitems].tag == ASN_ANY) {
         //don't add length bytes for ASN_ANY nor a tag
         continue;
      }
   *nsubitems += nsubs;
      
   if (len >= 0xFFFFFFFF)
     return (E_ASN_BAD_LEN);
   else if (len <= 0x7F)
     lenbytes = 1;       /* short form */
   else
     {
     lenbytes = 1;   //first byte = 0x80 + lenbytes
      while (len) {
        lenbytes++;
        len = len >> 8;
        }
     }

   /* add length length bytes */
   *l += lenbytes;
   /* tag 1 byte for now */
   *l += 1;
   }

*nsubitems += item->nsubitems;

return (0);
}
