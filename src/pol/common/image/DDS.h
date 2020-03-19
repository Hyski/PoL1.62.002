#pragma once

//=====================================================================================//
//                                      class DDS                                      //
//=====================================================================================//
class DDS
{
	VFile *m_image;
	DDSURFACEDESC2 m_ddesc;

public:
	DDS(VFile *img);

	bool Valid() const { return m_image != 0; }
	const DDSURFACEDESC2 &GetDesc() const { return m_ddesc; }
	VFile *GetFile() const { return m_image; }
};