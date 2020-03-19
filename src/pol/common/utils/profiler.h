#ifndef _PROFILER_HEADER_
#define _PROFILER_HEADER_


class CodeProfiler
	{
	public:
		static void FrameStart();   //��� �������� �������/����
		static void Init();
		static void Close();
		
	public:
		CodeProfiler(char* BlockName);
		virtual ~CodeProfiler();

	private://����� ������
		enum {IDENT_LEN=32};
		struct Item
			{
			float OverallTime;
			int   FrameNumber;
			float FrameTime;
			int BlockNumber;
			Item():OverallTime(0.f),FrameNumber(0),BlockNumber(0){};
			};
		static std::map<std::string,Item> m_ProfData;
	private:
	 static void DepictItem(const std::string &name,const Item& it);

	private://�� ������ ���������
		float m_StartTime;
		char  m_Name[IDENT_LEN];

	};


#endif