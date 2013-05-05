#include "Database/GDBCSQLStorage.h"
#include "Database/SQLStorageImpl.h"

void SQLStorage::EraseEntry(GDBC::uint32 id)
{
    GDBC::uint32 offset = 0;
    for(GDBC::uint32 x = 0; x < iNumFields; ++x)
    {
        switch(dst_format[x])
        {
            case FT_LOGIC:
                offset += sizeof(bool);   break;
            case FT_BYTE:
                offset += sizeof(char);   break;
            case FT_INT:
                offset += sizeof(GDBC::uint32); break;
            case FT_FLOAT:
                offset += sizeof(float);  break;
            case FT_STRING:
            {
                if(pIndex[id])
                    delete [] *(char**)((char*)(pIndex[id])+offset);

                offset += sizeof(char*);
                break;
            }
            case FT_NA:
            case FT_NA_BYTE:
                break;
            case FT_IND:
            case FT_SORT:
                assert(false && "SQL storage not have sort field types");
                break;
            default:
                assert(false && "unknown format character");
                break;
        }
    }

    pIndex[id] = NULL;
}

void SQLStorage::Free ()
{
    GDBC::uint32 offset = 0;
    for(GDBC::uint32 x = 0; x < iNumFields; ++x)
    {
        switch(dst_format[x])
        {
            case FT_LOGIC:
                offset += sizeof(bool);   break;
            case FT_BYTE:
                offset += sizeof(char);   break;
            case FT_INT:
                offset += sizeof(GDBC::uint32); break;
            case FT_FLOAT:
                offset += sizeof(float);  break;
            case FT_STRING:
            {
                for(GDBC::uint32 y = 0; y < MaxEntry; ++y)
                    if(pIndex[y])
                        delete [] *(char**)((char*)(pIndex[y])+offset);

                offset += sizeof(char*);
                break;
            }
            case FT_NA:
            case FT_NA_BYTE:
                break;
            case FT_IND:
            case FT_SORT:
                assert(false && "SQL storage not have sort field types");
                break;
            default:
                assert(false && "unknown format character");
                break;
        }
    }

    delete [] pIndex;
    delete [] data;
}

void SQLStorage::Load()
{
    SQLStorageLoader loader;
    loader.Load(*this);
}
