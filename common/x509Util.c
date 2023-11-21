#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "x509Util.h"
#include "asn1.hpp"
#include "log.hpp"
#include "memory.h"

char* getCertSubjectName(unsigned char* cert, size_t l_cert)
{
   int ret = 0;
   ASN1_ITEM subject;
   static char dn[256];
   size_t l_dn = 0;
   char *p_dn = NULL;

   if ((l_cert == 0) || (cert == NULL)) {
	   return ("&lt;empty&gt;");
   }

   memset(dn, 0, 256);
   ret = asn1_get_item(cert, l_cert, X509_SUBJECT, &subject);
   if (ret) {
      strcpy(dn, "error retrieving subject name from certificate");
      return (dn);
   }

   ret = get_printable_dn(&subject, &p_dn, &l_dn);
   if (ret) {
      strcpy(dn, "error decoding DN");
      return (dn);
   }

   memcpy(dn, p_dn, 255 < l_dn?255:l_dn); //leave one byte for nullterm

   if (p_dn) {
      free(p_dn);
      p_dn = 0;
   }

   return dn;
}

   
int isEndEntity(char* cert, size_t l_cert)
{
   int ret = 0;
   ASN1_ITEM main;
   ASN1_ITEM extensions, extension, oid, item;
   char path[] = "\1\1";
//   unsigned int oidList[20];
//   int l_oidList = 20;
//   char *dump = 0;

   if ((l_cert == 0) || (cert == NULL)) {
	   return (0);
   }

   ret = asn1_get_item((unsigned char*) cert, l_cert, "\1\1", &main);
   if (ret == E_ASN_ITEM_NOT_FOUND) {
      return 0; //should never happen here
   }
   
   //dump_asn1(main.p_data,main.l_data, 2, &dump);
   //log_info(dump);
   
   ret = asn1_find_item(main.p_data, main.l_data, ASN_SE(3), &extensions);
   if (ret == E_ASN_ITEM_NOT_FOUND) {
      return 1; //most probably an end entity
   }

#define OID_BASIC_CONSTRAINTS      "\x55\x1D\x13"
   //loop through all extensions
   for (int i = 1; ; i++) {
      
      path[1] = i;

      ret = asn1_get_item(extensions.p_data, extensions.l_data, path, &extension);
      if (ret == E_ASN_ITEM_NOT_FOUND) {
         return 1; //most probably an end entity
      }
      ret = asn1_get_item(extension.p_data, extension.l_data, "\1", &oid);
      if (ret == E_ASN_ITEM_NOT_FOUND) {
         return 1; //most probably an end entity
      }
      
//      ret = asn1_dec_oid(oid.p_data, oid.l_data, oidList, &l_oidList);
//      log_info("extension->oid: %s", oid2str(oid.p_data, oid.l_data));

      if ((oid.l_data == 3) && (memcmp(oid.p_data, OID_BASIC_CONSTRAINTS, 3) == 0)) {

         //"2.5.29.19" => basic constraints telling us if this is a CA
         //OCTETSTRING->SEQ->
         ret = asn1_find_item(extension.p_data, extension.l_data, ASN_OCTET_STRING, &item);
         if (ret == E_ASN_ITEM_NOT_FOUND) {
            return 1; //error: most probably an end entity
         }
         if (item.l_data == 0) {
            //this is an end entity
            return 1;
         }
         ret = asn1_find_item(item.p_data, item.l_data, ASN_SEQUENCE, &item);
         if (ret == E_ASN_ITEM_NOT_FOUND) {
            return 1; //error: most probably an end entity
         }
         ret = asn1_find_item(item.p_data, item.l_data, ASN_BOOLEAN, &item);
         if (ret == E_ASN_ITEM_NOT_FOUND) {
            return 1; //default false => end entity
         }
         if ((item.l_data == 1) && (item.p_data[0] == ASN_TRUE)) {
            return 0;  //this is a CA
         }
         return 1; //this is an end-entity
      }
   }
   
   return 0;
}


unsigned int getKeyUsage(char* cert, size_t l_cert)
{
   int ret = 0;
   ASN1_ITEM main;
   ASN1_ITEM extensions, extension, oid, item;
   char path[] = "\1\1";
   
   if ((l_cert == 0) || (cert == NULL)) {
	   return (0);
   }

   ret = asn1_get_item((unsigned char*) cert, l_cert, "\1\1", &main);
   if (ret == E_ASN_ITEM_NOT_FOUND) {
      return 0; //should never happen here
   }

   ret = asn1_find_item(main.p_data, main.l_data, ASN_SE(3), &extensions);
   if (ret == E_ASN_ITEM_NOT_FOUND) {
      return 1; //most probably an end entity
   }
   
#define OID_KEYUSAGE      "\x55\x1D\x0F"
   //loop through all extensions
   for (int i = 1; ; i++) {
      
      path[1] = i;
      
      ret = asn1_get_item(extensions.p_data, extensions.l_data, path, &extension);
      if (ret == E_ASN_ITEM_NOT_FOUND) {
         return 0;
      }
      ret = asn1_get_item(extension.p_data, extension.l_data, "\1", &oid);
      if (ret == E_ASN_ITEM_NOT_FOUND) {
         return 0;
      }
      
      if ((oid.l_data == 3) && (memcmp(oid.p_data, OID_KEYUSAGE, 3) == 0)) {
         
         //"2.5.29.15" => keyusage
         ret = asn1_find_item(extension.p_data, extension.l_data, ASN_OCTET_STRING, &item);
         if (ret == E_ASN_ITEM_NOT_FOUND) {
            return 0; //error
         }
         
         ret = asn1_find_item(item.p_data, item.l_data, ASN_BIT_STRING, &item);
         if (ret == E_ASN_ITEM_NOT_FOUND) {
            return 0; //error
         }
         unsigned int test = asn_bitstring2ui(item.p_data, item.l_data);
         return test;
      }
   }
   
   return 0;
}

size_t getRSAKeyLength(const char *cert, size_t l_cert)
{
   int ret = 0;
   ASN1_ITEM spki, keyalg, key;

   ret = asn1_get_item((unsigned char*) cert, l_cert, "\1\1\7\1", &spki);
   if (ret == E_ASN_ITEM_NOT_FOUND) {
      // could not find SubjectPublicKeyInfo
      return 0;
   }

   ret = asn1_get_item(spki.p_data, spki.l_data, "\1\1" , &keyalg);
   if (ret == E_ASN_ITEM_NOT_FOUND) {
      // key algo not found
      return 0;
   }

   if (keyalg.l_data != 9 || memcmp(keyalg.p_data, "\x2a\x86\x48\xf7\x0d\x01\x01\x01", 9) != 0) {
      // is not an RSA key
      return 0;
   }
   ret = asn1_get_item(spki.p_data, spki.l_data, "\2\1\1", &key);
   if (ret == E_ASN_ITEM_NOT_FOUND) {
      // RSA key bits not found
      return 0;
   }
   return key.l_data;
}

char* getValidUntil(char* cert, size_t l_cert)
{
   int ret = 0;
   static char time[LEN_DATE+1];
   ASN1_ITEM  item;

   if ((l_cert == 0) || (cert == NULL)) {
      return (0);
   }

   ret = asn1_get_item((unsigned char*)cert, l_cert, X509_VALID_UNTIL, &item);
   if (ret) {
      return(NULL);
   }

   if(item.tag == ASN_UTCTIME) {
      /* if the value is greater than 50 asume the last century acc. to RFC 2459 */
      if( (item.p_data[0]-0x30 + item.p_data[1]-0x30) >= 50)
         memcpy(time, "19", 2);
      else
         memcpy(time, "20", 2);
      memcpy(time + 2, item.p_data, item.l_data-1);  // without 'Z'
   }
   else if(item.tag == ASN_GENERALIZEDTIME)
   {
      memcpy(time, item.p_data, item.l_data-1);  // without 'Z'
   }
   else
      return(NULL);
   
   time[LEN_DATE] = 0;
   return(time);
}


int isTimeBeforeNow (int len, char *p_atime)
{
   //int fraction_digits = 0;
   int century         = 0;
   int pos             = 0;
   struct tm ts;
   time_t now;
   struct tm *p_tNow = NULL;
   time_t t;
   
   memset(&ts, 0, sizeof(ts));
   
   while(pos < len)
   {
      // check if fraction is given
      if (p_atime[pos] == '.')
      {
         switch (pos)
         {
            case 14:
               century = 1;  // determine whether century ("20") is given
            case 12:
               //fraction_digits = len - pos - 1; // get length of fraction
               break;
               
            default:
               return -1;//E_X509_TIME_WRONG_FORMAT;
         }
         break;
      }
      
      if(++pos == len)
      {
         // no fraction
         switch (pos)
         {
            case 14:
               century = 1;
            case 12:
               //fraction_digits = 0;
               break;
            default:
               return -2;//E_X509_TIME_WRONG_FORMAT;
         }
      }
   }
   
   if (century)
   {
      ts.tm_year =  asc2uchar(*p_atime++) * 1000;
      ts.tm_year += asc2uchar(*p_atime++) * 100;
   }
   else
      ts.tm_year = 2000;
   
   ts.tm_year += asc2uchar(*p_atime++) * 10;
   ts.tm_year += asc2uchar(*p_atime++);
   /* tm_year starts from 0 (=1900) */
   ts.tm_year -= 1900;
   
   ts.tm_mon  = asc2uchar(*p_atime++) * 10;
   ts.tm_mon += asc2uchar(*p_atime++);
   /* tm_mon starts from 0 */
   ts.tm_mon -= 1;
   
   ts.tm_mday  = asc2uchar(*p_atime++) * 10;
   ts.tm_mday += asc2uchar(*p_atime++);
   
   ts.tm_hour  = asc2uchar(*p_atime++) * 10;
   ts.tm_hour += asc2uchar(*p_atime++);
   
   ts.tm_min  = asc2uchar(*p_atime++) * 10;
   ts.tm_min += asc2uchar(*p_atime++);
   
   ts.tm_sec  = asc2uchar(*p_atime++) * 10;
   ts.tm_sec += asc2uchar(*p_atime++);
   
   /* get setting for dst see tm_isdst*/
   time(&now);
   p_tNow = localtime(&now);

   ts.tm_isdst = p_tNow->tm_isdst;
   t = mktime(&ts);
   
   //mktime(gmtime(&t));
   /* input is utc, since mktime converts from local to gmt, we have to add the difference again */
   /* to get utc time */
   t +=  mktime(localtime(&t)) - mktime(gmtime(&t));
   
   //compare with now
   if (t > now) {
      return 1;
   }
   return -1;
}

int getKeyInfo(const unsigned char *cert, size_t l_cert, int *keyType, size_t *keySize)
{
   int ret = 0;
   ASN1_ITEM keyinfo, keyalg, key;

   ret = asn1_get_item((unsigned char*) cert, l_cert, X509_KEYINFO, &keyinfo);
   if (ret == E_ASN_ITEM_NOT_FOUND) {
      // could not find SubjectPublicKeyInfo
      return ret;
   }

   ret = asn1_get_item(keyinfo.p_data, keyinfo.l_data, "\1\1" , &keyalg);
   if (ret == E_ASN_ITEM_NOT_FOUND) {
      return ret;
   }

   if (keyalg.tag != ASN_OID) {
      return E_ASN_ITEM_NOT_FOUND;
   }
 
   char *oid = oid2str(keyalg.p_data, keyalg.l_data);
   if (strcmp(oid, "1.2.840.113549.1.1.1") == 0) {
      *keyType = X509_KEYTYPE_RSA;

      ret = asn1_get_item(keyinfo.p_data, keyinfo.l_data, "\2\1\1", &key);
      if (ret == E_ASN_ITEM_NOT_FOUND) {
         return ret;
      }
      *keySize = key.l_data;
   }
   else if (strcmp(oid, "1.2.840.10045.2.1") == 0) {
      *keyType = X509_KEYTYPE_EC;
      ret = asn1_get_item(keyinfo.p_data, keyinfo.l_data, "\2", &key);
      if (ret == E_ASN_ITEM_NOT_FOUND) {
         return ret;
      }
#pragma message ("review this since this might not be 100% correct XXXXXX")
      *keySize = key.l_data - 1; // -1 byte for unused bits in BITSRING
   }
   
   return ret;
}
