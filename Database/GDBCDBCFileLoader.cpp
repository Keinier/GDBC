#include "Database/GDBCDBCFileLoader.h"

DBCFileLoader::DBCFileLoader()
{
    data = NULL;
    fieldsOffset = NULL;
}

bool DBCFileLoader::Load(const char *filename, const char *fmt)
{

    GDBC::uint32 header;
    if(data)
    {
        delete [] data;
        data=NULL;
    }

    FILE * f=fopen(filename,"rb");
    if(!f)return false;

    if(fread(&header,4,1,f)!=1)                             // Number of records
        return false;

    EndianConvert(header);
    if(header!=0x43424457)
        return false;                                       //'WDBC'

    if(fread(&recordCount,4,1,f)!=1)                        // Number of records
        return false;

    EndianConvert(recordCount);

    if(fread(&fieldCount,4,1,f)!=1)                         // Number of fields
        return false;

    EndianConvert(fieldCount);

    if(fread(&recordSize,4,1,f)!=1)                         // Size of a record
        return false;

    EndianConvert(recordSize);

    if(fread(&stringSize,4,1,f)!=1)                         // String size
        return false;

    EndianConvert(stringSize);

    fieldsOffset = new GDBC::uint32[fieldCount];
    fieldsOffset[0] = 0;
    for(GDBC::uint32 i = 1; i < fieldCount; i++)
    {
        fieldsOffset[i] = fieldsOffset[i - 1];
        if (fmt[i - 1] == 'b' || fmt[i - 1] == 'X')         // byte fields
            fieldsOffset[i] += 1;
        else                                                // 4 byte fields (int32/float/strings)
            fieldsOffset[i] += 4;
    }

    data = new unsigned char[recordSize*recordCount+stringSize];
    stringTable = data + recordSize*recordCount;

    if(fread(data,recordSize*recordCount+stringSize,1,f)!=1)
        return false;

    fclose(f);
    return true;
}

DBCFileLoader::~DBCFileLoader()
{
    if(data)
        delete [] data;
    if(fieldsOffset)
        delete [] fieldsOffset;
}

DBCFileLoader::Record DBCFileLoader::getRecord(size_t id)
{
    assert(data);
    return Record(*this, data + id*recordSize);
}

GDBC::uint32 DBCFileLoader::GetFormatRecordSize(const char * format,GDBC::int32* index_pos)
{
    GDBC::uint32 recordsize = 0;
    GDBC::int32 i = -1;
    for(GDBC::uint32 x = 0; format[x]; ++ x)
    {
        switch(format[x])
        {
            case FT_FLOAT:
                recordsize += sizeof(float);
                break;
            case FT_INT:
                recordsize += sizeof(GDBC::uint32);
                break;
            case FT_STRING:
                recordsize += sizeof(char*);
                break;
            case FT_SORT:
                i=x;
                break;
            case FT_IND:
                i=x;
                recordsize += sizeof(GDBC::uint32);
                break;
            case FT_BYTE:
                recordsize += sizeof(GDBC::uint8);
                break;
            case FT_LOGIC:
                assert(false && "DBC files not have logic field type");
                break;
            case FT_NA:
            case FT_NA_BYTE:
                break;
            default:
                assert(false && "unknown format character");
                break;
        }
    }

    if (index_pos)
        *index_pos = i;

    return recordsize;
}

char* DBCFileLoader::AutoProduceData(const char* format, GDBC::uint32& records, char**& indexTable)
{
    /*
    format STRING, NA, FLOAT,NA,INT <=>
    struct{
    char* field0,
    float field1,
    int field2
    }entry;

    this func will generate  entry[rows] data;
    */

    typedef char * ptr;
    if(strlen(format)!=fieldCount)
        return NULL;

    //get struct size and index pos
    GDBC::int32 i;
    GDBC::uint32 recordsize=GetFormatRecordSize(format,&i);

    if(i>=0)
    {
        GDBC::uint32 maxi=0;
        //find max index
        for(GDBC::uint32 y=0;y<recordCount;y++)
        {
            GDBC::uint32 ind=getRecord(y).getUInt (i);
            if(ind>maxi)maxi=ind;
        }

        ++maxi;
        records=maxi;
        indexTable=new ptr[maxi];
        memset(indexTable,0,maxi*sizeof(ptr));
    }
    else
    {
        records = recordCount;
        indexTable = new ptr[recordCount];
    }

    char* dataTable= new char[recordCount*recordsize];

    GDBC::uint32 offset=0;

    for(GDBC::uint32 y =0; y < recordCount; ++y)
    {
        if (i >= 0)
        {
            indexTable[getRecord(y).getUInt(i)]=&dataTable[offset];
        }
        else
            indexTable[y]=&dataTable[offset];

        for(GDBC::uint32 x = 0; x < fieldCount; ++x)
        {
            switch(format[x])
            {
                case FT_FLOAT:
                    *((float*)(&dataTable[offset]))=getRecord(y).getFloat(x);
                    offset += sizeof(float);
                    break;
                case FT_IND:
                case FT_INT:
                    *((GDBC::uint32*)(&dataTable[offset]))=getRecord(y).getUInt(x);
                    offset += sizeof(GDBC::uint32);
                    break;
                case FT_BYTE:
                    *((GDBC::uint8*)(&dataTable[offset]))=getRecord(y).getUInt8(x);
                    offset += sizeof(GDBC::uint8);
                    break;
                case FT_STRING:
                    *((char**)(&dataTable[offset]))=NULL;   // will be replaces non-empty or "" strings in AutoProduceStrings
                    offset += sizeof(char*);
                    break;
                case FT_LOGIC:
                    assert(false && "DBC files not have logic field type");
                    break;
                case FT_NA:
                case FT_NA_BYTE:
                case FT_SORT:
                    break;
                default:
                    assert(false && "unknown format character");
                    break;
            }
        }
    }

    return dataTable;
}

char* DBCFileLoader::AutoProduceStrings(const char* format, char* dataTable)
{
    if(strlen(format)!=fieldCount)
        return NULL;

    char* stringPool= new char[stringSize];
    memcpy(stringPool,stringTable,stringSize);

    GDBC::uint32 offset=0;

    for(GDBC::uint32 y =0; y < recordCount; ++y)
    {
        for(GDBC::uint32 x = 0; x < fieldCount; ++x)
        {
            switch(format[x])
            {
                case FT_FLOAT:
                    offset += sizeof(float);
                    break;
                case FT_IND:
                case FT_INT:
                    offset += sizeof(GDBC::uint32);
                    break;
                case FT_BYTE:
                    offset += sizeof(GDBC::uint8);
                    break;
                case FT_STRING:
                {
                    // fill only not filled entries
                    char** slot = (char**)(&dataTable[offset]);
                    if(!*slot || !**slot)
                    {
                        const char * st = getRecord(y).getString(x);
                        *slot=stringPool+(st-(const char*)stringTable);
                    }
                    offset += sizeof(char*);
                    break;
                }
                case FT_LOGIC:
                    assert(false && "DBC files not have logic field type");
                    break;
                case FT_NA:
                case FT_NA_BYTE:
                case FT_SORT:
                    break;
                default:
                    assert(false && "unknown format character");
                    break;
            }
        }
    }

    return stringPool;
}
