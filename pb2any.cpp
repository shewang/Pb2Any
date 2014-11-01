/*
 * =====================================================================================
 *
 *       Filename:  protobufxjson.cpp
 *
 *    Description:  protobuf and json convert
 *
 *        Version:  1.0
 *        Created:  Friday, May 03, 2013 04:41:37 HKT
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  shewangliu, 
 *   Organization:  tencent
 *
 * =====================================================================================
 */
 
#include "pb2any.h"
#include <sstream> 
#include <iostream>
#define ENCODE_PRIMITIVE_TYPE(TYPE, CPPTYPE_METHOD)      \
        case FieldDescriptor::CPPTYPE_##TYPE: \
        {\
            valueToBinary(buf, pReflection->Get##CPPTYPE_METHOD(message, pFieldDescriptor));\
            break;\
        }

#define ENCODE_REPEATED_TYPE(TYPE, CPPTYPE_METHOD)      \
        case FieldDescriptor::CPPTYPE_##TYPE: \
        {\
            valueToBinary(buf, pReflection->GetRepeated##CPPTYPE_METHOD(message, pFieldDescriptor, FieldNum));\
            break;\
        }

#define DECODE_PRIMITIVE_TYPE(TYPE, OPT, CPP_TYPE, CPPTYPE_METHOD)      \
        case FieldDescriptor::CPPTYPE_##TYPE: \
        {\
            pReflection->OPT##CPPTYPE_METHOD(&message, pFieldDescriptor, (CPP_TYPE)value);\
            break;\
        }

void valueToBinary(std::string& sBuf, uint64_t value)
{
	if (value < 0xFF)
	{
		sBuf.push_back((char) Integer1);
		sBuf.push_back((char) value);
	}
	else if (value <= 0xFFFF)
	{
		sBuf.push_back((char) Integer2);
		uint16_t wTmp = htons(static_cast<uint16_t> (value));
		sBuf.append(reinterpret_cast<char*> (&wTmp), sizeof(wTmp));
	}
	else if (value <= 0xFFFFFFFF)
	{
		sBuf.push_back((char) Integer4);
		uint32_t dwTmp = htonl(static_cast<uint32_t> (value));
		sBuf.append(reinterpret_cast<char*> (&dwTmp), sizeof(dwTmp));
	}
	else
	{
		sBuf.push_back((char) Integer8);
		uint64_t ui64Tmp = htonll(value);
		sBuf.append(reinterpret_cast<char*> (&ui64Tmp), sizeof(ui64Tmp));
	}
}

void valueToBinary(std::string& sBuf, const std::string& value)
{
	if (value.size() <= 0xFF)
	{
		sBuf.push_back((char) String1);
		sBuf.push_back(static_cast<unsigned char>(value.size()));
	}
	else if (value.size() <= 0xFFFF)
	{
		sBuf.push_back((char) String2);
		uint16_t wSize =
			htons(static_cast<uint16_t> (value.size()));
		sBuf.append(reinterpret_cast<char*> (&wSize), sizeof(wSize));
	}
	else
	{
		sBuf.push_back((char) String4);
		uint32_t dwSize = htonl(static_cast<uint32_t> (value.size()));
		sBuf.append(reinterpret_cast<char*> (&dwSize), sizeof(dwSize));
	}
	sBuf.append(value.data(), value.size());
}

bool Pb2Any::encodeRepeatedValue(string& buf, const FieldDescriptor *pFieldDescriptor, const Message& message, int32 FieldNum)
{
    const Reflection *pReflection = message.GetReflection();
	
	bool ret = true;
    switch(pFieldDescriptor->cpp_type())
    {
        ENCODE_REPEATED_TYPE(INT32, Int32);
        ENCODE_REPEATED_TYPE(UINT32, UInt32);
        ENCODE_REPEATED_TYPE(INT64, Int64);
        ENCODE_REPEATED_TYPE(UINT64, UInt64);
        ENCODE_REPEATED_TYPE(STRING, String);

        case FieldDescriptor::CPPTYPE_BOOL:
        {
            valueToBinary(buf, (pReflection->GetRepeatedBool(message, pFieldDescriptor, FieldNum) ? 1 : 0));
            break;
        }
        case FieldDescriptor::CPPTYPE_ENUM:
        {
			EnumValueDescriptor *pEnumValueDes = NULL;
            pEnumValueDes = (EnumValueDescriptor *)pReflection->GetRepeatedEnum(message, pFieldDescriptor, FieldNum);
            valueToBinary(buf, pEnumValueDes->number());
            break;
        }
        case FieldDescriptor::CPPTYPE_DOUBLE:
        {
            double value = pReflection->GetRepeatedDouble(message, pFieldDescriptor, FieldNum);
            std::ostringstream sstream;
            sstream << value;
            valueToBinary(buf, sstream.str());
            break;
        }
        case FieldDescriptor::CPPTYPE_FLOAT:
        {
            float value = pReflection->GetRepeatedFloat(message, pFieldDescriptor, FieldNum);
            std::ostringstream sstream;
            sstream << value;
            valueToBinary(buf, sstream.str());
            break;
        }
        case FieldDescriptor::CPPTYPE_MESSAGE:
        {
            ret = encodeMessage(buf, pReflection->GetRepeatedMessage(message, pFieldDescriptor, FieldNum));
            break;
        }
        default:
        {
            m_errCode = EC_UNKNOWN_TYPE;
            ret = false;
			break;
        }
    }

    return ret;
}

bool Pb2Any::encodeField(string& buf, const FieldDescriptor *pFieldDescriptor, const Message& message)
{
	bool ret = true;
    const Reflection *pReflection = message.GetReflection();
    switch(pFieldDescriptor->cpp_type())
    {
        ENCODE_PRIMITIVE_TYPE(INT32, Int32);
        ENCODE_PRIMITIVE_TYPE(UINT32, UInt32);
        ENCODE_PRIMITIVE_TYPE(INT64, Int64);
        ENCODE_PRIMITIVE_TYPE(UINT64, UInt64);
        ENCODE_PRIMITIVE_TYPE(STRING, String);

        case FieldDescriptor::CPPTYPE_BOOL:
        {
            valueToBinary(buf, (pReflection->GetBool(message, pFieldDescriptor) ? 1 : 0));
            break;
        }
        case FieldDescriptor::CPPTYPE_ENUM:
        {
            const EnumValueDescriptor *pEnumValueDes = NULL;
            pEnumValueDes = (EnumValueDescriptor *)pReflection->GetEnum(message, pFieldDescriptor);
            valueToBinary(buf, pEnumValueDes->number());
            break;
        }
        case FieldDescriptor::CPPTYPE_DOUBLE:
        {
            double value = pReflection->GetDouble(message, pFieldDescriptor);
            std::ostringstream sstream;
            sstream << value;
            valueToBinary(buf, sstream.str());
            break;
        }
        case FieldDescriptor::CPPTYPE_FLOAT:
        {
            float value = pReflection->GetFloat(message, pFieldDescriptor);
            std::ostringstream sstream;
            sstream << value;
            valueToBinary(buf, sstream.str());
            break;
        }
        case FieldDescriptor::CPPTYPE_MESSAGE:
        {
            const Message& tmp = pReflection->GetMessage(message, pFieldDescriptor);
            ret = encodeMessage(buf, tmp);
              
            break;
        }
        default:
        {
            m_errCode = EC_UNKNOWN_TYPE;
            ret = false;
			break;
        }
    }

    return ret;
}

bool Pb2Any::encodeRepeatedField(string& buf, const FieldDescriptor *pFieldDescriptor, const Message& message)
{
    const Reflection *pReflection = message.GetReflection();

    buf.push_back((char) Vector);

    uint32_t dwSize = htonl(pReflection->FieldSize(message, pFieldDescriptor));
    buf.append(reinterpret_cast<char*> (&dwSize), sizeof(dwSize));

    int32  FieldNum;
	bool ret = true;
    for(FieldNum = 0; FieldNum < pReflection->FieldSize(message, pFieldDescriptor); FieldNum++)
    {
        if (!(ret = encodeRepeatedValue(buf, pFieldDescriptor, message, FieldNum)))
        {
            break;
        }
    }

    return ret;
}

bool Pb2Any::encodeMessage(string& buf, const Message& message)
{
    const Reflection *pReflection = message.GetReflection();

    const FieldDescriptor *pFieldDescriptor = NULL;
    bool bRepeated;
    //bool bRequired;
    string name_str;

	bool ret = true;

    buf.push_back((char)Map);
    size_t dwSizePos = buf.size();
    uint32_t size = 0;
    buf.append(reinterpret_cast<char*> (&size), sizeof(size));

    std::vector<const FieldDescriptor *> fields;
    pReflection->ListFields(message, &fields);
    //std::cout << "size=" << fields.size() << ",dsd=" << message.GetDescriptor()->field_count() << std::endl;
    for(size_t i = 0; i < fields.size(); i++)
    {
        pFieldDescriptor = fields[i];
        bRepeated = pFieldDescriptor->is_repeated();
        /*bRequired = pFieldDescriptor->is_required();
        //std::cout << "required="<< bRequired << std::endl;
        if (bRequired && !pReflection->HasField(message, pFieldDescriptor))
        {
        	ret = false;
			break;
        }
		*/
        if (!bRepeated && !pReflection->HasField(message, pFieldDescriptor))
        {
            continue;
        }
		
        if (bRepeated && !pReflection->FieldSize(message, pFieldDescriptor))
        {
            continue;
        }

        size++;
        if (pFieldDescriptor->is_extension())
        {
            name_str = pFieldDescriptor->full_name();
        }
        else
        {
            name_str = pFieldDescriptor->name();
        }

        //std::cout << "name="<< name_str << std::endl;
        buf.push_back(name_str.size());
        buf.append(name_str.data(), name_str.size());
        if(bRepeated)
        {
            if (!(ret = encodeRepeatedField(buf, pFieldDescriptor, message)))
            {
                m_errMsg = "encode " + name_str + " failed.";
				break;
            }
            continue;
        }
        
        if (!(ret = encodeField(buf, pFieldDescriptor, message)))
        {
            m_errMsg = "encode " + name_str + " failed.";
            break;
        }
    }

    size = htonl(size);
    memcpy((char*) buf.data() + dwSizePos, reinterpret_cast<char*> (&size), sizeof(size));
    return ret;
}

bool Pb2Any::encode(string& buf,const Message& message)
{
    buf = "";
    buf.push_back((char)0);

    if (!encodeMessage(buf, message))
    {
        buf.clear();
        return false;
    }

    return true;
}

bool Any2Pb::decode(const string& buf,  Message& message)
{
   begin= buf.c_str();
   end = begin + buf.length();
    if (!parse(message))
    {
        message.Clear();
        return false;
    }

    return true;
}

bool Any2Pb::decode(const char * pszData, size_t size, Message& message)
{
    begin= pszData;
    end = begin + size;
    if(!parse(message))
    {
        message.Clear();
        return false;
    }

    return true;
}

bool Any2Pb::parse(Message& message)
{
	current = begin;

	unlikely(*current++ != 0) 
    {
        m_errMsg = "buf not begin with \'0\'";
        m_errCode = EC_WRONG_FORMAT;
        return false;
    }
	if (current == end) return true;
    unlikely(*current ++ != Map) 
    {
        m_errMsg = "the first element is not map";
        m_errCode = EC_WRONG_ELEMENT;
        return false;
    }
    uint32_t size = ntohl(*(uint32_t *)current);
    unlikely(0 == size)
        return true;

    current += sizeof(uint32_t);
	return readValue(size, message);
}

bool Any2Pb::readValue(unsigned long size, Message& message)
{        
    std::string name;
    const Descriptor *pDescriptor = message.GetDescriptor();
    const Reflection *pReflection = message.GetReflection();
    for (unsigned long i = 0; i < size; i++)
    {
        uint8_t ucNameLen =  *(uint8_t *)current; 
		current++;
		unlikely (current > end) 
        {
            m_errMsg = "elements too less";
            m_errCode = EC_WRONG_FORMAT;
            return false;
        }
        name.assign(current, ucNameLen);
		current += ucNameLen;
		unlikely (current > end) 
        {
            m_errMsg = name + " len is not right";
            m_errCode = EC_WRONG_FORMAT;
            return false;
        }

		unsigned char type;
        unsigned long size;
    	unlikely(!readNode(type, size)) return false;
		
        const FieldDescriptor *pFieldDescriptor = pDescriptor->FindFieldByName(name);
        if (!pFieldDescriptor)
		{
			pFieldDescriptor = pReflection->FindKnownExtensionByName(name);
    	}

		if (!pFieldDescriptor)
        {
			current += size;
        	continue;
		}
        
        if (!(pFieldDescriptor->is_repeated()) && (Vector == type))
        {
            m_errMsg = name + " should not be a vector";
            m_errCode = EC_WRONG_TYPE;
            return false;
        }

        if (pFieldDescriptor->is_repeated())
        {
            if (Vector != type)
            {
                m_errMsg = name + " should be a vector";
                m_errCode = EC_WRONG_TYPE;
                return false;
            }
            if(!readVector(size, pFieldDescriptor, message))
            {
                m_errMsg = name + " decode failed";
                return false;
            }
            continue;
        }

        if(!readValue(type, size, pFieldDescriptor, message))
        {
            m_errMsg = name + " decode failed";
            return false;
        }
    }

    if (!message.IsInitialized())
    {
        m_errMsg = "missed some required field(s), " + message.InitializationErrorString();
        m_errCode = EC_MISS_REQUIRED;
        return false;
    }

    return true;
}

bool Any2Pb::readValue(unsigned char type, unsigned long size, const FieldDescriptor *pFieldDescriptor, Message& message)
{
	bool successful = false;
   
	switch (type)
	{
        case Integer1:
        case Integer2:
        case Integer4:
        case Integer8:
            successful = readInteger(size, pFieldDescriptor, message);
            break;
        case String1:
        case String2:
        case String4:
            successful = readString(size, pFieldDescriptor, message);
            break;
        case Map:
            successful = readMap(size, pFieldDescriptor, message);
            break;
        default:
            m_errCode = EC_UNKNOWN_TYPE;
            successful = false;
            break;
	}

	return successful;
}

bool Any2Pb::readNode(unsigned char& type, unsigned long& size)
{
	type = *(unsigned char *)current;
	current++;
	unlikely (current > end)
    {
        m_errMsg = "wrong format";
        m_errCode = EC_WRONG_FORMAT;
        return false;
    }

	switch (type)
    {
        case Integer1:
            size = 1;
            break;
        case Integer2:
            size = 2;
            break;
        case Integer4:
            size = 4;
            break;
        case Integer8:
            size = 8;
            break;
        case String1:
            size = *(uint8_t *) current;
            current += 1;
            break;
        case String2:
            size = ntohs(*(uint16_t *) current);
            current += 2;
            break;
        case String4:
            size = ntohl(*(uint32_t *) current);
            current += 4;
            break;
        case Vector:
            size = ntohl(*(uint32_t *) current);
            current += 4;
            break;
        case Map:
            size = ntohl(*(uint32_t *) current);
            current += 4;
            break;
        default:
            m_errMsg = "unknown type";
            m_errCode = EC_UNKNOWN_TYPE;
            return false;
    }

	unlikely (current > end) return false;

	return true;
}

bool Any2Pb::readIntegerValue(unsigned long size,  uint64_t& value)
{
    switch(size)
	{
    	case 1:
    	{
    		value = (uint8_t) *(char *)current;
    		break;
    	}
    	case 2:
    	{
    		value = (uint16_t)ntohs(*(uint16_t *)current);
    		break;
    	}
    	case 4:
    	{
    		value = (uint32_t)ntohl(*(uint32_t *)current);
    		break;
    	}
    	case 8:
    	{
    		value = (uint64_t)ntohll(*(uint64_t *)current);
    		break;
    	}
    	default:
        {
            m_errCode = EC_WRONG_LENGTH;
    		return false;
        }
	}

    return true;
}

bool Any2Pb::readInteger(unsigned long size, const FieldDescriptor *pFieldDescriptor, Message&  message)
{
    uint64_t value;
    if (!readIntegerValue(size, value))
    {
        return false;
    }

    const Reflection *pReflection = message.GetReflection();
    switch(pFieldDescriptor->cpp_type())
    {
        DECODE_PRIMITIVE_TYPE(INT32, Set, int32_t, Int32);
        DECODE_PRIMITIVE_TYPE(UINT32, Set, uint32_t, UInt32);
        DECODE_PRIMITIVE_TYPE(INT64, Set, int64_t, Int64);
        DECODE_PRIMITIVE_TYPE(UINT64, Set, uint64_t, UInt64);

        case FieldDescriptor::CPPTYPE_ENUM:
        {
            EnumDescriptor      *pEnumDes = NULL;
            EnumValueDescriptor *pEnumValueDes = NULL;
            if ((pEnumDes = (EnumDescriptor *)pFieldDescriptor->enum_type()) == NULL)
            {
                m_errCode = EC_WRONG_ENUM;
                return false;
            }

            if ((pEnumValueDes = (EnumValueDescriptor *)pEnumDes->FindValueByNumber((int)value)) == NULL)
            {
                m_errCode = EC_WRONG_ENUM_TYPE;
                return false;
            }

            pReflection->SetEnum(&message, pFieldDescriptor, pEnumValueDes);
            break;
        }
        case FieldDescriptor::CPPTYPE_BOOL:
        {
            bool bValue = (value == 0) ? false:true;
            pReflection->SetBool(&message, pFieldDescriptor, bValue);
            break;
        }
        default:
        {
            m_errCode = EC_UNKNOWN_TYPE;
            return false;
        }
    }
    
    current += size;
	unlikely(current > end) 
    {
        m_errCode = EC_WRONG_FORMAT;
        return false;
    }

    return true;
}

bool Any2Pb::readString(unsigned long size, const FieldDescriptor *pFieldDescriptor, Message& message)
{
	unlikely (current + size > end) 
    {
        m_errCode = EC_WRONG_FORMAT;
        return false;
    }
    const Reflection *pReflection = message.GetReflection();
    switch(pFieldDescriptor->cpp_type())
    {
        case FieldDescriptor::CPPTYPE_STRING:
        {
            pReflection->SetString(&message, pFieldDescriptor, std::string(current, size));
            break;
        }
        case FieldDescriptor::CPPTYPE_DOUBLE:
        {
            double value = strtod(std::string(current, size).c_str(), NULL); 
            pReflection->SetDouble(&message, pFieldDescriptor, value);
            break;
        }
        case FieldDescriptor::CPPTYPE_FLOAT:
        {
            float value = (float)strtod(std::string(current, size).c_str(), NULL); 
            pReflection->SetFloat(&message, pFieldDescriptor, value);
            break;
        }
        default:
        {
            m_errCode = EC_UNKNOWN_TYPE;
            return false;
        }
    }
	current += size;
	return true;
}

bool Any2Pb::readMap(unsigned long size, const FieldDescriptor *pFieldDescriptor, Message& message)
{
    if (FieldDescriptor::CPPTYPE_MESSAGE != pFieldDescriptor->cpp_type())
    {
        m_errCode = EC_UNKNOWN_TYPE;
        return false;
    }
    const Reflection *pReflection = message.GetReflection();
    Message *pmessage = pReflection->MutableMessage(&message, pFieldDescriptor);

    return readValue(size, *pmessage);
}

bool Any2Pb::readRepeatedInteger(unsigned long size, const FieldDescriptor *pFieldDescriptor, Message& message)
{
    uint64_t value;
	
    if (!readIntegerValue(size, value))
    {
        return false;
    }
    const Reflection *pReflection = message.GetReflection();
    switch(pFieldDescriptor->cpp_type())
    {
        DECODE_PRIMITIVE_TYPE(INT32, Add, int32_t, Int32);
        DECODE_PRIMITIVE_TYPE(UINT32, Add, uint32_t, UInt32);
        DECODE_PRIMITIVE_TYPE(INT64, Add, int64_t, Int64);
        DECODE_PRIMITIVE_TYPE(UINT64, Add, uint64_t, UInt64);

        case FieldDescriptor::CPPTYPE_ENUM:
        {
            EnumDescriptor      *pEnumDes = NULL;
            EnumValueDescriptor *pEnumValueDes = NULL;
            if ((pEnumDes = (EnumDescriptor *)pFieldDescriptor->enum_type()) == NULL)
            {
                m_errCode = EC_WRONG_ENUM;
                return false;
            }

            if ((pEnumValueDes = (EnumValueDescriptor *)pEnumDes->FindValueByNumber((int)value)) == NULL)
            {
                m_errCode = EC_WRONG_ENUM_TYPE;
                return false;
            }

            pReflection->AddEnum(&message, pFieldDescriptor, pEnumValueDes);
            break;
        }
        case FieldDescriptor::CPPTYPE_BOOL:
        {
            bool bValue = (value == 0) ? false:true;
            pReflection->AddBool(&message, pFieldDescriptor, bValue);
            break;
        }
        default:
        {
            m_errCode = EC_UNKNOWN_TYPE;
            return false;
        }
    }
    
    current += size;
	unlikely(current > end) 
    {
        m_errCode = EC_WRONG_FORMAT;
        return false;
    }

    return true;
}

bool Any2Pb::readRepeatedString(unsigned long size, const FieldDescriptor *pFieldDescriptor, Message& message)
{
	unlikely (current + size > end) 
    {
        m_errCode = EC_WRONG_FORMAT;
        return false;
    }

    const Reflection *pReflection = message.GetReflection();
    switch(pFieldDescriptor->cpp_type())
    {
        case FieldDescriptor::CPPTYPE_STRING:
        {
            pReflection->AddString(&message, pFieldDescriptor, std::string(current, size));
            break;
        }
        case FieldDescriptor::CPPTYPE_DOUBLE:
        {
            double value = strtod(std::string(current, size).c_str(), NULL); 
            pReflection->AddDouble(&message, pFieldDescriptor, value);
            break;
        }
        case FieldDescriptor::CPPTYPE_FLOAT:
        {
            float value = (float)strtod(std::string(current, size).c_str(), NULL); 
            pReflection->AddFloat(&message, pFieldDescriptor, value);
            break;
        }
        default:
        {
            m_errCode = EC_UNKNOWN_TYPE;
            return false;
        }
    }
	current += size;
	return true;
}

bool Any2Pb::readRepeatedMap(unsigned long size, const FieldDescriptor *pFieldDescriptor, Message&  message)
{
    if (FieldDescriptor::CPPTYPE_MESSAGE != pFieldDescriptor->cpp_type())
    {
        m_errCode = EC_UNKNOWN_TYPE;
        return false;
    }
    
    const Reflection *pReflection = message.GetReflection();
    Message *pmessage = pReflection->AddMessage(&message, pFieldDescriptor);

    return readValue(size, *pmessage);
}

bool Any2Pb::readRepeatedValue(unsigned char type, unsigned long size, const FieldDescriptor *pFieldDescriptor, Message& message)
{
    bool successful = false;
   
	switch (type)
	{
	case Integer1:
	case Integer2:
	case Integer4:
	case Integer8:
		successful = readRepeatedInteger(size, pFieldDescriptor, message);
		break;
	case String1:
	case String2:
	case String4:
		successful = readRepeatedString(size, pFieldDescriptor, message);
		break;
	case Map:
		successful = readRepeatedMap(size, pFieldDescriptor, message);
		break;
	default:
        m_errCode = EC_UNKNOWN_TYPE;
		return false;
	}

	return successful;
}

bool Any2Pb::readVector(unsigned long size, const FieldDescriptor *pFieldDescriptor, Message& message)
{
    unsigned char current_type;
    unsigned long field_size;
	for (uint32_t index = 0; index < size; index++)
	{
    	unlikely(!readNode(current_type, field_size)) return false;
    	if (!readRepeatedValue(current_type, field_size, pFieldDescriptor, message))
        {
            return false;
        }
	}
	return true;
}

