#ifndef BRIEFCASE_H
#define BRIEFCASE_H

    #include <vcl.h>
    #include <windows.h>
    #include <math.h>
    #include <mem.h>

	// ������ �� �������� "W8012 Comparing signed and unsigned values"
	#pragma warn -csu

	// ������ �� �������� "W8032 Temporary used for parameter '???' in call to '???'"
	#pragma warn -lvc

    // ������ �� �������� "W8004 'identifier' is assigned a value that is never used"
    #pragma warn -aus
    

    // ��������� ����������
    struct SInvestment
    {
        char *name;               // �������� ����������
        int cost;                 // ���������
        int profit;               // �������
    };

    unsigned int ITER_COUNT=1;
    DWORD DeltaTime=0;

    #define STD_MAX_COST 100


/**************************************
           CBriefcase
***************************************/
    class CBriefcase
    {
        protected:
            SInvestment *invest;                      // ����������
            int NumItem;                              // ����� ���������� � ��������
            int MaxNumItem;                           // ������������ ����� ���������� � ��������
            int MaxCost;                              // ������ ��������

            int NoUsedProfit;                         // ��������� �������

            bool *best;                               // ������ �������
            int BestCost;                             // ��������� ������� �������
            int BestProfit;

            bool *cur;                                // ������� �������
            int CurCost;                              // ��������� �������� �������
            int CurProfit;

            bool *trial;                              // ������� ������� � ���������� ����������������� ���������
            int TrialCost;                            // ��������� �������� �������
            int TrialProfit;                          // ��� ���������� ����������������� ���������

            virtual void search()=0;                  // ����������� ����������, �� ���������

            void copy(SInvestment * &s, SInvestment * &d,  bool del, BYTE inc=0, bool create=true); // ����������� �������� ����������
            void copybool(bool * &s, bool * &d, BYTE inc=0, bool create=true);
            void resize();                            // ���������� ������� ������� �� 1
        public:
            CBriefcase(int count=0, int maxcost=STD_MAX_COST); // ����������� � ��������� ���-�� ���������� � ������������ ���������
            CBriefcase(const CBriefcase &v);          // ����������� �����������
            ~CBriefcase();

            bool add(char *a, int c, int p);          // ���������� ����������
            bool del(char *n);						  // �������� ����������
            bool del(unsigned int num);				  // �������� ����������
            void clear();                             // ������� ��������
            bool exists(char *n);					  // �������� �� ������������� ���������� � ��������� ������

            void setMaxCost(int maxcost);             // ��������� ������������ ���������
            int getMaxCost();
            int getNumItem();

            virtual void copyfrom(const CBriefcase &v); // ����� �������

            virtual void calcBestVariant();             // ���������� ������� ��������, �����

            friend void outbestBriefcase(const CBriefcase &v);  // ����� ����������� ��������
            friend void outBriefcase();                         // ����� ������ ����������
            friend void saveBriefcase(char *path);              // ���������� � ��������
            friend void loadBriefcase(char *path);              // ������ ����������
    };

//######################################################################################################################


/**************************************
           CBrunchAndBound
***************************************/
    class CBrunchAndBound : public CBriefcase
    {
        protected:
            virtual void search();
            void _search(int);
        public:
            CBrunchAndBound(int count=0, int maxcost=STD_MAX_COST);
            CBrunchAndBound(const CBrunchAndBound &v);      
    };


/**************************************
           CHillClimbing
***************************************/
    class CHillClimbing : public CBriefcase
    {
        protected:
            virtual void search();
        public:
            CHillClimbing(int count=0, int maxcost=STD_MAX_COST);
            CHillClimbing(const CHillClimbing &v);        
    };
    

/**************************************
           CRandomSearch
***************************************/
    class CRandomSearch : public CBriefcase
    {
        protected:
            bool _Add();    // ���������� ���������� �������� � ������������ �������
            bool _Del();    // �������� ���������� �������� �� ������������ �������
        public:
            CRandomSearch(int count=0, int maxcost=STD_MAX_COST);
            CRandomSearch(const CRandomSearch &v);        
    };


/**************************************
           CSimulatedAnnealing
***************************************/
    class CSimulatedAnnealing : public CRandomSearch
    {
        protected:
            virtual void search();
            int MaxUnchanged;
            int MaxSaved;
            int K_ch;
        public:
            CSimulatedAnnealing(int count=0, int maxcost=STD_MAX_COST);
            CSimulatedAnnealing(const CSimulatedAnnealing &v);

            void setMaxUnchanged(int);
            int getMaxUnchanged();
            void setMaxSaved(int);
            int getMaxSaved();
            void setK_changed(int);
            int getK_changed();
    };
    

/**************************************
           CLocalOptimum
***************************************/
    class CLocalOptimum : public CRandomSearch
    {
        protected:
            virtual void search();

            /*
                ������������� ��������� num ��������� ��� ��������� ������������ �������.
                ��������� ����������� �� ��� ���, ���� � ������� max_nochanged_test
              ��������� �� ���������� ���������.
                � ������� ������� ��������� �������� ��������� ���������, ���� � �������
              max_nobest_changes ��������� �� ����� ���������.
            */

            int num;
            int max_bad_tests;
            int max_bad_changes;

        public:
            CLocalOptimum(int count=0, int maxcost=STD_MAX_COST);
            CLocalOptimum(const CLocalOptimum &v);

            void setNum(int);
            void setMaxBadTests(int);
            void setMaxBadChanges(int);
            int getNum();
            int getMaxBadTests();
            int getMaxBadChanges();
    };

//######################################################################################################################

/**************************************
           CBalancedProfit
**************************************/
    class CBalancedProfit : public CBriefcase
    {
        protected:
            virtual void search();
        public:
            CBalancedProfit(int count=0, int maxcost=STD_MAX_COST);
            CBalancedProfit(const CBalancedProfit &v);
    };


/**************************************
           CRandomSearchMod
**************************************/
    class CRandomSearchMod : public CRandomSearch
    {
        protected:
            virtual void search();
        public:
            CRandomSearchMod(int count=0, int maxcost=STD_MAX_COST);
            CRandomSearchMod(const CRandomSearchMod &v);
    };


/**************************************
           CIncrementalImprovement
**************************************/
    class CIncrementalImprovement : public CRandomSearch
    {
        protected:
            virtual void search();
        public:
            CIncrementalImprovement(int count=0, int maxcost=STD_MAX_COST);
            CIncrementalImprovement(const CIncrementalImprovement &v);
    };


/**************************************
               CStopMoment
**************************************/
    class CStopMoment : public CRandomSearch
    {
        protected:
            virtual void search();
        public:
            CStopMoment(int count=0, int maxcost=STD_MAX_COST);
            CStopMoment(const CStopMoment &v);
    };



#endif // BRIEFCASE_H
