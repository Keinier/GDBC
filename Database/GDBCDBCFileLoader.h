#ifndef __GDBC_GDBCDBCFILE_LOADER_H__
#define __GDBC_GDBCDBCFILE_LOADER_H__

#include "GDBCPrerequisities.h"
#include "GDBCByteConverter.h"
#include <cassert>

enum
{
    FT_NA='x',                                              //not used or unknown, 4 byte size
    FT_NA_BYTE='X',                                         //not used or unknown, byte
    FT_STRING='s',                                          //char*
    FT_FLOAT='f',                                           //float
    FT_INT='i',                                             //uint32
    FT_BYTE='b',                                            //uint8
    FT_SORT='d',                                            //sorted by this field, field is not included
    FT_IND='n',                                             //the same,but parsed to data
    FT_LOGIC='l'                                            //Logical (boolean)
};

class DBCFileLoader
{
    public:
        DBCFileLoader();
        ~DBCFileLoader();

        bool Load(const char *filename, const char *fmt);

        class Record
        {
            public:
                float getFloat(size_t field) const
                {
                    assert(field < file.fieldCount);
                    float val = *reinterpret_cast<float*>(offset+file.GetOffset(field));
                    EndianConvert(val);
                    return val;
                }
                GDBC::uint32 getUInt(size_t field) const
                {
                    assert(field < file.fieldCount);
                    GDBC::uint32 val = *reinterpret_cast<GDBC::uint32*>(offset+file.GetOffset(field));
                    EndianConvert(val);
                    return val;
                }
                GDBC::uint8 getUInt8(size_t field) const
                {
                    assert(field < file.fieldCount);
                    return *reinterpret_cast<GDBC::uint8*>(offset+file.GetOffset(field));
                }

                const char *getString(size_t field) const
                {
                    assert(field < file.fieldCount);
                    size_t stringOffset = getUInt(field);
                    assert(stringOffset < file.stringSize);
                    return reinterpret_cast<char*>(file.stringTable + stringOffset);
                }

            private:
                Record(DBCFileLoader &file_, unsigned char *offset_): offset(offset_), file(file_) {}
                unsigned char *offset;
                DBCFileLoader &file;

                friend class DBCFileLoader;

        };

        // Get record by id
        Record getRecord(size_t id);
        /// Get begin iterator over records

        GDBC::uint32 GetNumRows() const { return recordCount;}
        GDBC::uint32 GetCols() const { return fieldCount; }
        GDBC::uint32 GetOffset(size_t id) const { return (fieldsOffset != NULL && id < fieldCount) ? fieldsOffset[id] : 0; }
        bool IsLoaded() {return (data!=NULL);}
        char* AutoProduceData(const char* fmt, GDBC::uint32& count, char**& indexTable);
        char* AutoProduceStrings(const char* fmt, char* dataTable);
        static GDBC::uint32 GetFormatRecordSize(const char * format, GDBC::int32 * index_pos = NULL);
    private:

        GDBC::uint32 recordSize;
        GDBC::uint32 recordCount;
        GDBC::uint32 fieldCount;
        GDBC::uint32 stringSize;
        GDBC::uint32 *fieldsOffset;
        unsigned char *data;
        unsigned char *stringTable;
};
#endif