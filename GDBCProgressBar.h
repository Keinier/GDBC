#ifndef __GDBC_GDBCPROGRESSBAR_H__
#define __GDBC_GDBCPROGRESSBAR_H__

#include "GDBCPrerequisities.h"

class _GDBCAPIExport BarGoLink
{
    public:                                                 // constructors
        explicit BarGoLink(int row_count);
        explicit BarGoLink(GDBC::uint32 row_count);               // row_count < ACE_INT32_MAX
        explicit BarGoLink(GDBC::uint64 row_count);               // row_count < ACE_INT32_MAX
        ~BarGoLink();

    public:                                                 // modifiers
        void step();

        static void SetOutputState(bool on);
    private:
        void init(int row_count);

        static bool m_showOutput;                           // not recommended change with existed active bar
        static char const * const empty;
        static char const * const full;

        int rec_no;
        int rec_pos;
        int num_rec;
        int indic_len;
};
#endif //__GDBC_GDBCPROGRESSBAR_H__
