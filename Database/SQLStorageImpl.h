#ifndef __GDBC_GDBCSQLSTORAGE_IMPL_H__
#define __GDBC_GDBCSQLSTORAGE_IMPL_H__

#include "GDBCProgressBar.h"
#include "GDBCDBCFileLoader.h"

template<class T>
template<class S, class D>
void SQLStorageLoaderBase<T>::convert(GDBC::uint32 /*field_pos*/, S src, D &dst)
{
    dst = D(src);
}

template<class T>
void SQLStorageLoaderBase<T>::convert_str_to_str(GDBC::uint32 /*field_pos*/, char const *src, char *&dst)
{
    if(!src)
    {
        dst = new char[1];
        *dst = 0;
    }
    else
    {
        GDBC::uint32 l = strlen(src) + 1;
        dst = new char[l];
        memcpy(dst, src, l);
    }
}

template<class T>
template<class S>
void SQLStorageLoaderBase<T>::convert_to_str(GDBC::uint32 /*field_pos*/, S /*src*/, char * & dst)
{
    dst = new char[1];
    *dst = 0;
}

template<class T>
template<class D>
void SQLStorageLoaderBase<T>::convert_from_str(GDBC::uint32 /*field_pos*/, char const* /*src*/, D& dst)
{
    dst = 0;
}

template<class T>
template<class V>
void SQLStorageLoaderBase<T>::storeValue(V value, SQLStorage &store, char *p, GDBC::uint32 x, GDBC::uint32 &offset)
{
    T * subclass = (static_cast<T*>(this));
    switch(store.dst_format[x])
    {
        case FT_LOGIC:
            subclass->convert(x, value, *((bool*)(&p[offset])) );
            offset+=sizeof(bool);
            break;
        case FT_BYTE:
            subclass->convert(x, value, *((char*)(&p[offset])) );
            offset+=sizeof(char);
            break;
        case FT_INT:
            subclass->convert(x, value, *((GDBC::uint32*)(&p[offset])) );
            offset+=sizeof(GDBC::uint32);
            break;
        case FT_FLOAT:
            subclass->convert(x, value, *((float*)(&p[offset])) );
            offset+=sizeof(float);
            break;
        case FT_STRING:
            subclass->convert_to_str(x, value, *((char**)(&p[offset])) );
            offset+=sizeof(char*);
            break;
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

template<class T>
void SQLStorageLoaderBase<T>::storeValue(char const* value, SQLStorage &store, char *p, GDBC::uint32 x, GDBC::uint32 &offset)
{
    T * subclass = (static_cast<T*>(this));
    switch(store.dst_format[x])
    {
        case FT_LOGIC:
            subclass->convert_from_str(x, value, *((bool*)(&p[offset])) );
            offset+=sizeof(bool);
            break;
        case FT_BYTE:
            subclass->convert_from_str(x, value, *((char*)(&p[offset])) );
            offset+=sizeof(char);
            break;
        case FT_INT:
            subclass->convert_from_str(x, value, *((GDBC::uint32*)(&p[offset])) );
            offset+=sizeof(GDBC::uint32);
            break;
        case FT_FLOAT:
            subclass->convert_from_str(x, value, *((float*)(&p[offset])) );
            offset+=sizeof(float);
            break;
        case FT_STRING:
            subclass->convert_str_to_str(x, value, *((char**)(&p[offset])) );
            offset+=sizeof(char*);
            break;
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

template<class T>
void SQLStorageLoaderBase<T>::Load(SQLStorage &store, bool error_at_empty /*= true*/)
{
    GDBC::uint32 maxi;
    Field *fields;
    QueryResult *result  = 0;//WorldDatabase.PQuery("SELECT MAX(%s) FROM %s", store.entry_field, store.table);
    if(!result)
    {
        std::stringstream ss;
        
        ss <<"Error loading " << store.table << " table (not exist?)";
        std::cout << ss;//SYSTEM_LOG(ss.str());
        exit(1);                                            // Stop server at loading non exited table or not accessable table
    }

    maxi = (*result)[0].GetUInt32()+1;
    delete result;

    result = 0;//WorldDatabase.PQuery("SELECT COUNT(*) FROM %s", store.table);
    if(result)
    {
        fields = result->Fetch();
        store.RecordCount = fields[0].GetUInt32();
        delete result;
    }
    else
        store.RecordCount = 0;

    result = 0;//WorldDatabase.PQuery("SELECT * FROM %s", store.table);

    if(!result)
    {
        if (error_at_empty)
        {
            std::stringstream ss;
            ss << store.table << " table is empty!";
            std::cout << ss ;//SYSTEM_LOG(ss.str());
        }else{
            std::stringstream ss;
            ss << store.table << " table is empty!";
            std::cout << ss; // SYSTEM_LOG(ss.str());
        }

        store.RecordCount = 0;
        return;
    }

    GDBC::uint32 recordsize = 0;
    GDBC::uint32 offset = 0;

    if(store.iNumFields != result->GetFieldCount())
    {
        store.RecordCount = 0;
        std::stringstream ss;
       ss << "Error in " << store.table << " table, probably sql file format was updated (there should be " << store.iNumFields << "fields in sql)";
            std::cout << ss; //SYSTEM_LOG(ss.str());

        delete result;
       
        exit(1);                                            // Stop server at loading broken or non-compatible table.
    }

    //get struct size
    for(GDBC::uint32 x = 0; x < store.iNumFields; ++x)
    {
        switch(store.dst_format[x])
        {
            case FT_LOGIC:
                recordsize += sizeof(bool);   break;
            case FT_BYTE:
                recordsize += sizeof(char);   break;
            case FT_INT:
                recordsize += sizeof(GDBC::uint32); break;
            case FT_FLOAT:
                recordsize += sizeof(float);  break;
            case FT_STRING:
                recordsize += sizeof(char*);  break;
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

    char** newIndex=new char*[maxi];
    memset(newIndex,0,maxi*sizeof(char*));

    char * _data= new char[store.RecordCount *recordsize];
    GDBC::uint32 count = 0;
    BarGoLink bar(store.RecordCount);
    do
    {
        fields = result->Fetch();
        bar.step();
        char *p=(char*)&_data[recordsize*count];
        newIndex[fields[0].GetUInt32()]=p;

        offset=0;
        for(GDBC::uint32 x = 0; x < store.iNumFields; x++)
            switch(store.src_format[x])
            {
                case FT_LOGIC:
                    storeValue((bool)(fields[x].GetUInt32() > 0), store, p, x, offset); break;
                case FT_BYTE:
                    storeValue((char)fields[x].GetUInt8(), store, p, x, offset); break;
                case FT_INT:
                    storeValue((GDBC::uint32)fields[x].GetUInt32(), store, p, x, offset); break;
                case FT_FLOAT:
                    storeValue((float)fields[x].GetFloat(), store, p, x, offset); break;
                case FT_STRING:
                    storeValue((char const*)fields[x].GetString(), store, p, x, offset); break;
                case FT_NA:
                case FT_NA_BYTE:
                    break;
                case FT_IND:
                case FT_SORT:
                    assert(false && "SQL storage not have sort field types");
                    break;
                default:
                    assert(false && "unknown format character");
            }
        ++count;
    }while( result->NextRow() );

    delete result;

    store.pIndex = newIndex;
    store.MaxEntry = maxi;
    store.data = _data;
}

#endif
