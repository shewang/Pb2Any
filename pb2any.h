/*
 * =====================================================================================
 *
 *       Filename:  protobufxjson.h
 *
 *    Description:  protobuf and json convert
 *
 *        Version:  1.0
 *        Created:  Friday, May 03, 2013 04:41:37 HKT
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  shewangliu, 
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef __PROTOBUF_2_JSON_H_
#define __PROTOBUF_2_JSON_H_
#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>

#if __BYTE_ORDER == __BIG_ENDIAN
#define ntohl(x)    (x)       
#define ntohs(x)    (x)       
#define htonl(x)    (x)       
#define htons(x)    (x)       
#define ntohll(x)   (x)       
#define htonll(x)   (x)       
#else  
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define ntohl(x)    __bswap_32 (x)
#define ntohs(x)    __bswap_16 (x)
#define htonl(x)    __bswap_32 (x)
#define htons(x)    __bswap_16 (x)
#define ntohll(x)   __bswap_64 (x)
#define htonll(x)   __bswap_64 (x)
#endif
#endif

#if !defined(__GNUC__) || (__GNUC__ == 2 && __GNUC_MINOR__ < 96)
#define likely(x)       if((x))
#define unlikely(x)     if((x))
#else
#define likely(x)       if(__builtin_expect((x) != 0, 1))
#define unlikely(x)     if(__builtin_expect((x) != 0, 0))
#endif

using namespace google::protobuf;

enum BinaryValueType //realtype
{
	Integer1	= 0, ///< tiny int value (1字节) 
	Integer2	= 1, ///< small int value (2字节)
	Integer4 	= 2, ///< signed integer value(int32)(4字节)
	Integer8 	= 3, ///< big signed interger value(int64)(8字节)
	String1 	= 4, ///< string value  //1个字节表示长度
	String2 	= 5, ///< string value  //2个字节表示长度
	String4 	= 6, ///< string value  //4个字节表示长度
	Vector 		= 7, ///< array value (double list) 
	Map 		= 8, ///< object value (collection of name/value pairs).
	EXT 		= 9,
};

enum ErrCode
{
    EC_SUCC = 0,                       //encode or decode ok
    EC_UNKNOWN_TYPE = EC_SUCC + 1,     //unknown type
    EC_WRONG_FORMAT = EC_SUCC + 2,     //wrong format to parse
    EC_WRONG_ENUM = EC_SUCC + 3,       //wrong enum
    EC_WRONG_ENUM_TYPE = EC_SUCC + 4,  //enum type out of enum range
    EC_WRONG_ELEMENT = EC_SUCC + 5,    //first element is not a map
    EC_WRONG_TYPE = EC_SUCC + 6,       //type is not right
    EC_MISS_REQUIRED = EC_SUCC + 7,    //miss some required field
    EC_WRONG_LENGTH = EC_SUCC + 8,     //integeral length not right
};

void valueToBinary(std::string& sBuf, uint64_t value);
void valueToBinary(std::string& sBuf, const std::string& value);

class Pb2Any{
public:
    Pb2Any(){}
    ~Pb2Any(){}
    bool encode(string& buf,const Message& message);

    const string & GetErrMsg() const
    {
        return m_errMsg;
    }

    int32_t GetErrCode() const
    {
        return m_errCode;
    }
private:
    bool encodeRepeatedValue(string& buf, const FieldDescriptor *pFieldDescriptor, const Message& message, int32 FieldNum);
    bool encodeField(string& buf, const FieldDescriptor *pFieldDescriptor, const Message& message);
    bool encodeRepeatedField(string& buf, const FieldDescriptor *pFieldDescriptor, const Message& message);
    bool encodeMessage(string& buf,  const Message& message);
    Pb2Any(const Pb2Any& rsh);
    Pb2Any& operator=(const Pb2Any& rsh);
private:
    string m_errMsg;
    int32  m_errCode;
};

class Any2Pb{
public:
    Any2Pb(){}
    ~Any2Pb(){}
    bool decode(const string& buf,  Message&  message);
    bool decode(const char *pszData, size_t size, Message& message);
    const string & GetErrMsg() const
    {
        return m_errMsg;
    }

    int32_t GetErrCode() const
    {
        return m_errCode;
    }
private:
    bool parse(Message& message);
    bool readValue(unsigned long size, Message& message);    
    bool readValue(unsigned char type, unsigned long size, const FieldDescriptor *pFieldDescriptor, Message& message);
    bool readNode(unsigned char& type, unsigned long& size);
    bool readIntegerValue(unsigned long size,  uint64_t& value);
    bool readInteger(unsigned long size, const FieldDescriptor *pFieldDescriptor, Message&  message);
    bool readString(unsigned long size, const FieldDescriptor *pFieldDescriptor, Message& message);
    bool readMap(unsigned long size, const FieldDescriptor *pFieldDescriptor, Message&  message);
    bool readRepeatedInteger(unsigned long size, const FieldDescriptor *pFieldDescriptor, Message&  message);
    bool readRepeatedString(unsigned long size, const FieldDescriptor *pFieldDescriptor, Message& message);
    bool readRepeatedMap(unsigned long size, const FieldDescriptor *pFieldDescriptor, Message&  message);
    bool readRepeatedValue(unsigned char type, unsigned long size, const FieldDescriptor *pFieldDescriptor, Message& message);
    bool readVector(unsigned long size, const FieldDescriptor *pFieldDescriptor, Message&  message);
    Any2Pb(const Any2Pb& rsh);
    Any2Pb& operator=(const Any2Pb& rsh);
private:
    const char* begin;
    const char* end;
    const char* current;
    string m_errMsg;
    int32  m_errCode;
};

inline bool encode(string& buf,const Message& message, string& errmsg, int32_t& errcode)
{
    Pb2Any pb;
    if (!pb.encode(buf, message))
    {
        errmsg = pb.GetErrMsg();
        errcode = pb.GetErrCode();
        return false;
    }

    return true;
}
inline bool decode(const string& buf,  Message&  out_message, string& errmsg, int32_t& errcode)
{
    Any2Pb any;
    if (!any.decode(buf, out_message))
    {
        errmsg = any.GetErrMsg();
        errcode = any.GetErrCode();
        return false;
    }

    return true;
}

inline bool decode(const char *pszData, size_t size, Message& out_message, string& errmsg, int32_t& errcode)
{
    Any2Pb any;
    if (!any.decode(pszData, size, out_message))
    {
        errmsg = any.GetErrMsg();
        errcode = any.GetErrCode();
        return false;
    }

    return true;
}
#endif

