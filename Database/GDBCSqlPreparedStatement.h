/******************************************************************************
*                                                                             *
*       File Name: GDBCSqlPreparedStatement.h                                 *
*       Description: Define Database interface and sql connections class.     *
*                                                                             *
*       Author: Keinier Caboverde Ram�rez. <keinier@gmail.com>                *
*       Date: 03/05/2013 11:26 a.m.                                           *
*       File's Version: 1.3.1                                                 *
*                                                                             *
******************************************************************************/
#ifndef __GDBC_GDBCSQLPREPAREDSTATEMENTS_H__
#define __GDBC_GDBCSQLPREPAREDSTATEMENTS_H__

#include "GDBCPrerequisities.h"
#include <ace/TSS_T.h>
#include <vector>
#include <stdexcept>

class Database;
class SqlConnection;
class QueryResult;

union SqlStmtField
{
    bool boolean;
    GDBC::uint8 ui8;
    GDBC::int8 i8;
    GDBC::uint16 ui16;
    GDBC::int16 i16;
    GDBC::uint32 ui32;
    GDBC::int32 i32;
    GDBC::uint64 ui64;
    GDBC::int64 i64;
    float f;
    double d;
};

enum SqlStmtFieldType
{
    FIELD_BOOL,
    FIELD_UI8,
    FIELD_UI16,
    FIELD_UI32,
    FIELD_UI64,
    FIELD_I8,
    FIELD_I16,
    FIELD_I32,
    FIELD_I64,
    FIELD_FLOAT,
    FIELD_DOUBLE,
    FIELD_STRING,
    FIELD_NONE
};

//templates might be the best choice here
//but I didn't have time to play with them
class _GDBCAPIExport SqlStmtFieldData
{
    public:
        SqlStmtFieldData() : m_type(FIELD_NONE) { m_binaryData.ui64 = 0; }
        ~SqlStmtFieldData() {}

        template<typename T>
        SqlStmtFieldData(T param) { set(param); }

        template<typename T1>
        void set(T1 param1);

        //getters
        bool toBool() const { assert(m_type == FIELD_BOOL); return static_cast<bool>(m_binaryData.ui8); }
        GDBC::uint8 toUint8() const { assert(m_type == FIELD_UI8); return m_binaryData.ui8; }
        GDBC::int8 toInt8() const { assert(m_type == FIELD_I8); return m_binaryData.i8; }
        GDBC::uint16 toUint16() const { assert(m_type == FIELD_UI16); return m_binaryData.ui16; }
        GDBC::int16 toInt16() const { assert(m_type == FIELD_I16); return m_binaryData.i16; }
        GDBC::uint32 toUint32() const { assert(m_type == FIELD_UI32); return m_binaryData.ui32; }
        GDBC::int32 toInt32() const { assert(m_type == FIELD_I32); return m_binaryData.i32; }
        GDBC::uint64 toUint64() const { assert(m_type == FIELD_UI64); return m_binaryData.ui64; }
        GDBC::int64 toInt64() const { assert(m_type == FIELD_I64); return m_binaryData.i64; }
        float toFloat() const { assert(m_type == FIELD_FLOAT); return m_binaryData.f; }
        double toDouble() const { assert(m_type == FIELD_DOUBLE); return m_binaryData.d; }
        const char * toStr() const { assert(m_type == FIELD_STRING); return m_szStringData.c_str(); }

        // get type of data
        SqlStmtFieldType type() const { return m_type; }
        
		// get underlying buffer type
        void * buff() const { return m_type == FIELD_STRING ? (void * )m_szStringData.c_str() : (void *)&m_binaryData; }

        //get size of data
        size_t size() const
        {
            switch (m_type)
            {
                case FIELD_NONE:    return 0;
                case FIELD_BOOL:    //return sizeof(bool);
                case FIELD_UI8:     return sizeof(GDBC::uint8);
                case FIELD_UI16:    return sizeof(GDBC::uint16);
                case FIELD_UI32:    return sizeof(GDBC::uint32);
                case FIELD_UI64:    return sizeof(GDBC::uint64);
                case FIELD_I8:      return sizeof(GDBC::int8);
                case FIELD_I16:     return sizeof(GDBC::int16);
                case FIELD_I32:     return sizeof(GDBC::int32);
                case FIELD_I64:     return sizeof(GDBC::int64);
                case FIELD_FLOAT:   return sizeof(float);
                case FIELD_DOUBLE:  return sizeof(double);
                case FIELD_STRING:  return m_szStringData.length();

                default:
                    throw std::runtime_error("unrecognized type of SqlStmtFieldType obtained");
            }
        }

    private:
        SqlStmtFieldType m_type;
        SqlStmtField m_binaryData;
        std::string m_szStringData;
};

// template specialization
template<> inline void SqlStmtFieldData::set(bool val) { m_type = FIELD_BOOL; m_binaryData.ui8 = val; }
template<> inline void SqlStmtFieldData::set(GDBC::uint8 val) { m_type = FIELD_UI8; m_binaryData.ui8 = val; }
template<> inline void SqlStmtFieldData::set(GDBC::int8 val) { m_type = FIELD_I8; m_binaryData.i8 = val; }
template<> inline void SqlStmtFieldData::set(GDBC::uint16 val) { m_type = FIELD_UI16; m_binaryData.ui16 = val; }
template<> inline void SqlStmtFieldData::set(GDBC::int16 val) { m_type = FIELD_I16; m_binaryData.i16 = val; }
template<> inline void SqlStmtFieldData::set(GDBC::uint32 val) { m_type = FIELD_UI32; m_binaryData.ui32 = val; }
template<> inline void SqlStmtFieldData::set(GDBC::int32 val) { m_type = FIELD_I32; m_binaryData.i32 = val; }
template<> inline void SqlStmtFieldData::set(GDBC::uint64 val) { m_type = FIELD_UI64; m_binaryData.ui64 = val; }
template<> inline void SqlStmtFieldData::set(GDBC::int64 val) { m_type = FIELD_I64; m_binaryData.i64 = val; }
template<> inline void SqlStmtFieldData::set(float val) { m_type = FIELD_FLOAT; m_binaryData.f = val; }
template<> inline void SqlStmtFieldData::set(double val) { m_type = FIELD_DOUBLE; m_binaryData.d = val; }
template<> inline void SqlStmtFieldData::set(const char * val) { m_type = FIELD_STRING; m_szStringData = val; }

class SqlStatement;

// prepared statement executor
class _GDBCAPIExport SqlStmtParameters
{
    public:
        typedef std::vector<SqlStmtFieldData> ParameterContainer;

        // reserve memory to contain all input parameters of stmt
        explicit SqlStmtParameters(int nParams);

        ~SqlStmtParameters() {}

        // get amount of bound parameters
        int boundParams() const { return int(m_params.size()); }
        
		// add parameter
        void addParam(const SqlStmtFieldData& data) { m_params.push_back(data); }
        
		// empty SQL statement parameters. In case nParams > 1 - reserve memory for parameters
        // should help to reuse the same object with batched SQL requests
        void reset(const SqlStatement& stmt);
        
		// swaps contents of intenral param container
        void swap(SqlStmtParameters& obj);
        
		// get bound parameters
        const ParameterContainer& params() const { return m_params; }

    private:
        SqlStmtParameters& operator=(const SqlStmtParameters& obj);

        // statement parameter holder
        ParameterContainer m_params;
};

// statement ID encapsulation logic
class SqlStatementID
{
    public:
        SqlStatementID() : m_bInitialized(false) {}

        int ID() const { return m_nIndex; }
        int arguments() const { return m_nArguments; }
        bool initialized() const { return m_bInitialized; }

    private:
        friend class Database;
        void init(int nID, int nArgs) { m_nIndex = nID; m_nArguments = nArgs; m_bInitialized = true; }

        int m_nIndex;
        int m_nArguments;
        bool m_bInitialized;
};

// statement index
class _GDBCAPIExport SqlStatement
{
    public:
        ~SqlStatement() { delete m_pParams; }

        SqlStatement(const SqlStatement& index) : m_index(index.m_index), 
			m_pDB(index.m_pDB), 
			m_pParams(NULL)
        {
            if(index.m_pParams)
                m_pParams = new SqlStmtParameters(*(index.m_pParams));
        }

        SqlStatement& operator=(const SqlStatement& index);

        int ID() const { return m_index.ID(); }
        int arguments() const { return m_index.arguments(); }

        bool Execute();
        bool DirectExecute();

        // templates to simplify 1-4 parameter bindings
        template<typename ParamType1>
        bool PExecute(ParamType1 param1)
        {
            arg(param1);
            return Execute();
        }

        template<typename ParamType1, typename ParamType2>
        bool PExecute(ParamType1 param1, 
			ParamType2 param2)
        {
            arg(param1);
            arg(param2);
            return Execute();
        }

        template<typename ParamType1, 
			typename ParamType2, 
			typename ParamType3>
        bool PExecute(ParamType1 param1, 
		ParamType2 param2, 
		ParamType3 param3)
        {
            arg(param1);
            arg(param2);
            arg(param3);
            return Execute();
        }

        template<typename ParamType1, 
			typename ParamType2, 
			typename ParamType3, 
			typename ParamType4>
        bool PExecute(ParamType1 param1, ParamType2 param2,
		ParamType3 param3, ParamType4 param4)
        {
            arg(param1);
            arg(param2);
            arg(param3);
            arg(param4);
            return Execute();
        }

        //bind parameters with specified type
        void addBool(bool var) { arg(var); }
        void addUInt8(GDBC::uint8 var) { arg(var); }
        void addInt8(GDBC::int8 var) { arg(var); }
        void addUInt16(GDBC::uint16 var) { arg(var); }
        void addInt16(GDBC::int16 var) { arg(var); }
        void addUInt32(GDBC::uint32 var) { arg(var); }
        void addInt32(GDBC::int32 var) { arg(var); }
        void addUInt64(GDBC::uint64 var) { arg(var); }
        void addInt64(GDBC::int64 var) { arg(var); }
        void addFloat(float var) { arg(var); }
        void addDouble(double var) { arg(var); }
        void addString(const char * var) { arg(var); }
        void addString(const std::string& var) { arg(var.c_str()); }
        void addString(std::ostringstream& ss) { arg(ss.str().c_str()); ss.str(std::string()); }

    protected:
        // don't allow anyone except Database class to create static SqlStatement objects
        friend class Database;
        SqlStatement(const SqlStatementID& index, Database& db) : m_index(index), 
			m_pDB(&db), m_pParams(NULL) {}

    private:

        SqlStmtParameters * get()
        {
            if(!m_pParams)
                m_pParams = new SqlStmtParameters(arguments());

            return m_pParams;
        }

        SqlStmtParameters * detach()
        {
            SqlStmtParameters * p = m_pParams ? m_pParams : new SqlStmtParameters(0);
            m_pParams = NULL;
            return p;
        }

        // helper function
        // use appropriate add* functions to bind specific data type
        template<typename ParamType>
        void arg(ParamType val)
        {
            SqlStmtParameters * p = get();
            p->addParam(SqlStmtFieldData(val));
        }

        SqlStatementID m_index;
        Database * m_pDB;
        SqlStmtParameters * m_pParams;
};

// base prepared statement class
class _GDBCAPIExport SqlPreparedStatement
{
    public:
        virtual ~SqlPreparedStatement() {}

        bool isPrepared() const { return m_bPrepared; }
        bool isQuery() const { return m_bIsQuery; }

        GDBC::uint32 params() const { return m_nParams; }
        GDBC::uint32 columns() const { return isQuery() ? m_nColumns : 0; }

        // initialize internal structures of prepared statement
        // upon success m_bPrepared should be true
        virtual bool prepare() = 0;
        
		// bind parameters for prepared statement from parameter placeholder
        virtual void bind(const SqlStmtParameters& holder) = 0;

        //execute statement w/o result set
        virtual bool execute() = 0;

    protected:
        SqlPreparedStatement(const std::string& fmt, SqlConnection& conn) : m_szFmt(fmt), 
			m_nParams(0), m_nColumns(0), m_bPrepared(false), m_bIsQuery(false), m_pConn(conn) {}

        GDBC::uint32 m_nParams;
        GDBC::uint32 m_nColumns;
        bool m_bIsQuery;
        bool m_bPrepared;
        std::string m_szFmt;
        SqlConnection& m_pConn;
};

//prepared statements via plain SQL string requests
class _GDBCAPIExport SqlPlainPreparedStatement : public SqlPreparedStatement
{
    public:
        SqlPlainPreparedStatement(const std::string& fmt, SqlConnection& conn);
        ~SqlPlainPreparedStatement() {}

        //this statement is always prepared
        virtual bool prepare() { return true; }

        //we should replace all '?' symbols with substrings with proper format
        virtual void bind(const SqlStmtParameters& holder);

        virtual bool execute();

    protected:
        void DataToString(const SqlStmtFieldData& data, std::ostringstream& fmt);

        std::string m_szPlainRequest;
};

#endif
