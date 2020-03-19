#if !defined(__DOS_DATETIME_H_INCLUDED_2023399421164473__)
#define __DOS_DATETIME_H_INCLUDED_2023399421164473__



namespace mll
{

namespace ml_encrypted_zip
{


#pragma pack(push,1)
//=====================================================================================//
//                                   struct dos_date                                   //
//=====================================================================================//
struct dos_date
{
	unsigned short day : 5;
	unsigned short month : 4;
	unsigned short year : 7;
};

//=====================================================================================//
//                                   struct dos_time                                   //
//=====================================================================================//
struct dos_time
{
	unsigned short seconds : 5;
	unsigned short minutes : 6;
	unsigned short hours : 5;
};
#pragma pack(pop)


}

}

#endif // !defined(__DOS_DATETIME_H_INCLUDED_2023399421164473__)