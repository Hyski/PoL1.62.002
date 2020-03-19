/**************************************************************                 
                                                                                
                             Virtuality                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: Halo
                                                                                
                                                                                
   Author: Mikhail L. Lepakhin (Flif)
***************************************************************/                

#include "precomp.h"

#include "..\GraphPipe\GraphPipe.h"
#include "EffectInfo.h"
#include "HaloEffect.h"
#include "../../logic2/TraceUtils.h"

//
// ������� ����� ��� halo (�����������)
//

// ����������� � �����������
BaseHalo::BaseHalo(
				   const point3 rp,			     // ��������� �����
				   const std::string &Shader,    // ������ �������
				   const std::string &Sound,     // ��������� �����
				   const bool SoundMove          // ��������� �� ����
				   )
{
//	LOG("BaseHalo::BaseHalo begin\n");
	// ����, ������� ���������� ������ �� ��� ����� NextTick
	first_time = true;
	root_point = rp;

	vel = point3(0, 0, 0);  // ��������

	bbox.Degenerate();
	bbox.Enlarge(root_point - point3(0.00001, 0.00001, 0.00001));
	bbox.Enlarge(root_point + point3(0.00001, 0.00001, 0.00001));
	
	// ������
	SetShader(Shader);
	// ����
	if(Sound == "")
	{
		// ����� ���
		sound_object = new EmptySound();
	}
	else
	{
		if(SoundMove)
		{
			sound_object = new ActiveSound(Sound);
		}
		else
		{
			sound_object = new FixedSound(Sound, rp);
		}
	}
//	LOG("BaseHalo::BaseHalo end\n");
}

// ����������
BaseHalo::~BaseHalo()
{
	// ����
	delete sound_object;
}

// ���������� ������� ������� ����� � ������� ������ �����
bool BaseHalo::Update()
{
//	LOG("BaseHalo::Update begin\n");
	// ����
	sound_object->Update(root_point, vel);
	// ����������� ������ ��� ������������� (points_vector)
	points_vector[0] = root_point;
	// ����� ������� ������� �������������
	pGraphPipe->TransformPoints(1, points_vector, points2d_vector);
	// ���������� �����
	Multiply();
//	LOG("BaseHalo::Update end\n");
	return true;
}

// ������� ����������� �����
void BaseHalo::Multiply()
{
//	LOG("BaseHalo::Multiply begin\n");
	if (state.state == HaloState::hidden)
	{
		ready_num = 0;
		return;
	}
	else
	{
		ready_num = PEAK_NUM;
	}

	//float z = points2d_vector[0].z;
	//if(z == -1.0f)
	//{
	//	ready_num = 0;
	//	return;
	//}

	float f, n, betta;
	float xfactor;
	float yfactor;
	int xres, yres;
	pGraphPipe->GetProjOptions(&f, &n, &betta);
	pGraphPipe->GetResolution(&xres, &yres);
	point3 right = pGraphPipe->GetCam()->GetRight();
	point3 up = pGraphPipe->GetCam()->GetUp();

	xfactor = PoLFXFactor;
	yfactor = PoLFXFactor;

	point3 xdelta = right * size * xfactor;
	point3 ydelta = up * size * yfactor;

	prim.Verts[0].pos = points2d_vector[0] - xdelta + ydelta;
	prim.Verts[1].pos = points2d_vector[0] + xdelta + ydelta;
	prim.Verts[2].pos = points2d_vector[0] + xdelta - ydelta;
	prim.Verts[3].pos = points2d_vector[0] - xdelta + ydelta;
	prim.Verts[4].pos = points2d_vector[0] + xdelta - ydelta;
	prim.Verts[5].pos = points2d_vector[0] - xdelta - ydelta;

	prim.Verts[0].diffuse = color;
	prim.Verts[1].diffuse = color;
	prim.Verts[2].diffuse = color;
	prim.Verts[3].diffuse = color;
	prim.Verts[4].diffuse = color;
	prim.Verts[5].diffuse = color;
//	LOG("BaseHalo::Multiply end\n");
}

// ������� ������� ���������
void BaseHalo::UpdateVisibility(const float current_time)
{
	point3 camera = pGraphPipe->GetCam()->GetPos();
	bool camera_changed = pGraphPipe->GetCam()->IsViewChanged();

	///////////////////////////////////////////////////
	/////////////  HIDDEN or DISSAPPEARING
	///////////////////////////////////////////////////
	if( (state.state == HaloState::hidden) ||
		(state.state == HaloState::disappearing)
		)
	{
		if(state.state == HaloState::disappearing)
		{
			//DebugInfo::Add(100, 100, "halo: disappearing");
			// halo ������� � ������ ������� 
			if((current_time - state.prev_time) > state.disappearing_time)
			{
				// ������� ������� => ����������� � ��������� ���������
				state.prev_time = current_time;
				state.state = HaloState::hidden;
				return;
			}
			
			unsigned int r, g, b;
			b = RGB_GETBLUE(save_color);
			g = RGB_GETGREEN(save_color);
			r = RGB_GETRED(save_color);
			
			//DebugInfo::Add(100, 190, "r = %d, g = %d, b = %d", r, g, b);
			
			r = r * (1 - (current_time - state.prev_time)/state.disappearing_time);
			g = g * (1 - (current_time - state.prev_time)/state.disappearing_time);
			b = b * (1 - (current_time - state.prev_time)/state.disappearing_time);
			
			//DebugInfo::Add(100, 210, "r = %d, g = %d, b = %d", r, g, b);
			
			color = RGB_MAKE(r, g, b);
		}
		/*
		else DebugInfo::Add(100, 100, "halo: hidden");

		if(state.bbox_visible)
			DebugInfo::Add(100, 130, "halo bbox: visible");
		else
			DebugInfo::Add(100, 130, "halo bbox: not visible");
		if(state.trace_ray_visible)
			DebugInfo::Add(100, 160, "halo traceray: visible");
		else
			DebugInfo::Add(100, 160, "halo traceray: not visible");*/

		// halo �� ���� �����

		if((!camera_changed) && (!state.camera_was_changed))
		{
			// ������ �� ��������� � �� ��������������
			return;
		}
		
		// ����� ������ ���� ����������, ���� �����������

		// ��������, ��������� �� BBox halo � ������� ���������
		Frustum * fru = &pGraphPipe->GetCam()->Cone;
		if(fru->TestPoint(root_point) == Frustum::NOTVISIBLE)
		{
			// BBox �� �����
			if(state.bbox_visible)
			{
				// BBox ��� �����, � ������ ����� �� ���� ���������
				state.bbox_visible = false;
				// ���������� ��� disappearing
				if(state.state == HaloState::disappearing)
				{
					// ����� ����������� � ��������� ���������
					state.prev_time = current_time;
					state.state = HaloState::hidden;
				}
			}
			return;
		}

		// BBox �����
		if(state.bbox_visible)
		{
			// BBox ��� ����� � ������� �������
			if((current_time - state.prev_trace_time) > 0.2)
			{
				state.prev_trace_time = current_time;
				state.camera_was_changed = false;
				// ������������ ���
				ShotTracer tr(0, camera,
					(root_point - camera),
					(root_point - camera).Length(),
					ShotTracer::F_VISROOFS|ShotTracer::F_SKIP_SHIELDS|ShotTracer::F_SKIP_INVISIBLE_ENTS|ShotTracer::F_SIGHT);
				
				if(tr.GetMaterial() == ShotTracer::MT_AIR)
				{
					// ��� ����� � ������ => halo ���� ����� => ��������� � ����� ���������
					state.trace_ray_visible = true;
					state.state = HaloState::appearing;
					state.prev_time = current_time;
					state.prev_trace_time = current_time;
					color = 0;
				}
			}
			else
			{
				// ��� �� ����� ��������� trace ray, �� ������ ���������
				state.camera_was_changed = true;
			}
			return;
		}

		// BBox �� ��� �����, �� ���� �������
		state.bbox_visible = true;
		// ������������ ���
		ShotTracer tr(0, camera,
			(root_point - camera), (root_point - camera).Length(),
			ShotTracer::F_VISROOFS|ShotTracer::F_SKIP_SHIELDS|ShotTracer::F_SKIP_INVISIBLE_ENTS|ShotTracer::F_SIGHT);
		if(tr.GetMaterial() == ShotTracer::MT_AIR)
		{
			// ��� ����� � ������ => halo ���� ����� => ��������� � ����� ���������
			state.trace_ray_visible = true;
			state.prev_trace_time = current_time;
			state.state = HaloState::appearing;
			state.prev_time = current_time;
			color = 0;
		}
		else
		{
			// ��� ����� �� � ������
			state.prev_trace_time = current_time;
			state.trace_ray_visible = false;
		}
		return;
	}

	///////////////////////////////////////////////////
	/////////////  VISIBLE or APPEARING
	///////////////////////////////////////////////////
	if( (state.state == HaloState::visible) ||
		(state.state == HaloState::appearing)
		)
	{
		if(state.state == HaloState::appearing)
		{
			//DebugInfo::Add(100, 100, "halo: appearing");

			// halo ��������� � ���������� �������
			if((current_time - state.prev_time) > state.appearing_time)
			{
				// ������ �������
				state.prev_time = current_time;
				state.state = HaloState::visible;
				color = save_color;
				return;
			}
			
			unsigned int r, g, b;
			b = RGB_GETBLUE(save_color);
			g = RGB_GETGREEN(save_color);
			r = RGB_GETRED(save_color);
			
			//DebugInfo::Add(100, 190, "r = %d, g = %d, b = %d", r, g, b);
			
			r = r * (current_time - state.prev_time)/state.appearing_time;
			g = g * (current_time - state.prev_time)/state.appearing_time;
			b = b * (current_time - state.prev_time)/state.appearing_time;
			
			//DebugInfo::Add(100, 210, "r = %d, g = %d, b = %d", r, g, b);
			
			color = RGB_MAKE(r, g, b);
		}
		else 
		{
			unsigned int r, g, b;
			b = RGB_GETBLUE(save_color);
			g = RGB_GETGREEN(save_color);
			r = RGB_GETRED(save_color);
			color = RGB_MAKE(r, g, b);
		}
		
		/*
		if(state.bbox_visible)
			DebugInfo::Add(100, 130, "halo bbox: visible");
		else
			DebugInfo::Add(100, 130, "halo bbox: not visible");
		if(state.trace_ray_visible)
			DebugInfo::Add(100, 160, "halo traceray: visible");
		else
			DebugInfo::Add(100, 160, "halo traceray: not visible");*/

		// halo ���� �����
		if((!camera_changed) && (!state.camera_was_changed))
		{
			// ������ �� ��������� � �� ��������������
			return;
		}
		
		// ��������, ��������� �� BBox halo � ������� ���������
		Frustum * fru = &pGraphPipe->GetCam()->Cone;
		if(fru->TestPoint(root_point) == Frustum::NOTVISIBLE)
		{
			// BBox �� ����� => halo ������ ��������� �����
			state.bbox_visible = false;
			state.prev_time = current_time;
			state.state = HaloState::hidden;
			return;

		}
		
		// BBox ��� ����� � ������� �������
		if((current_time - state.prev_trace_time) > 0.2)
		{
			state.prev_trace_time = current_time;
			state.camera_was_changed = false;
			// ������������ ���
			ShotTracer tr(0, camera,
				(root_point - camera),
				(root_point - camera).Length(),
				ShotTracer::F_VISROOFS|ShotTracer::F_SKIP_SHIELDS|ShotTracer::F_SKIP_INVISIBLE_ENTS|ShotTracer::F_SIGHT);
			
			if(tr.GetMaterial() == ShotTracer::MT_AIR)
			{
				// ��� ����� � ������ => halo ��-�������� �����
				state.prev_trace_time = current_time;
			}
			else
			{
				// ��� ����� �� � ������ => halo �������� ���� �������
				state.prev_time = current_time;
				state.prev_trace_time = current_time;
				state.state = HaloState::disappearing;
				state.trace_ray_visible = false;
			}
		}
		else
		{
			// ��� �� ����� ��������� trace ray, �� ������ ���������
			state.camera_was_changed = true;
		}
		return;		
	}
}

//
// ����� simple halo
//

// ����������� � �����������
SimpleHaloEffect::SimpleHaloEffect(
								   const point3 rp,			  // ��������� �����
								   const point3& Color,
								   const SIMPLE_HALO_EFFECT_INFO &info
								   ): BaseHalo(rp, info.Shader, info.Sound, info.SndMov)
{
//	LOG("SimpleHaloEffect::SimpleHaloEffect begin\n");
	// �������� �������� �����, ������ � ���������� ���������
	ready_num = PEAK_NUM;
	// ������� �������
	//color_vector = new unsigned int [PEAK_NUM];
	//texcoord_vector = new texcoord [PEAK_NUM];
	//ready_vector = new point3 [PEAK_NUM];
	prim.Verts.resize(PEAK_NUM);
	points2d_vector = new point3 [1];
	points_vector = new point3 [1];

	size = info.Size;

	unsigned int c1 = Color.x * 255.0;
	unsigned int c2 = Color.y * 255.0;
	unsigned int c3 = Color.z * 255.0;
	color = RGB_MAKE(c1, c2, c3);
	save_color = color;
	//LOG("SimpleHaloEffect::SimpleHaloEffect end\n");
}

// ����������
SimpleHaloEffect::~SimpleHaloEffect()
{
	//delete[] color_vector;
	//delete[] texcoord_vector;
	//delete[] ready_vector;
	delete[] points2d_vector;
	delete[] points_vector;
}

// ��������� ��� �������
// (������ ������ ���������)
bool SimpleHaloEffect::NextTick(const float current_time)
{
//	LOG("SimpleHaloEffect::NextTick begin\n");
	if(first_time) { first_time = false; Start(current_time); }

	UpdateVisibility(current_time);

	return true;
}

// � ������ ����� ������ ���������� ������
// ������� ��� ������� �������
bool SimpleHaloEffect::Start(const float start_time)
{
//	LOG("SimpleHaloEffect::Start begin\n");
	// ����������� ���������� ����������
	prim.Verts[0].uv = texcoord(0, 1);
	prim.Verts[1].uv = texcoord(1, 1);
	prim.Verts[2].uv = texcoord(1, 0);
	prim.Verts[3].uv = texcoord(0, 1);
	prim.Verts[4].uv = texcoord(1, 0);
	prim.Verts[5].uv = texcoord(0, 0);
	
	prim.Verts[0].pos.z = 0;
	prim.Verts[1].pos.z = 0;
	prim.Verts[2].pos.z = 0;
	prim.Verts[3].pos.z = 0;
	prim.Verts[4].pos.z = 0;
	prim.Verts[5].pos.z = 0;

	state.prev_time = start_time;
	state.state = HaloState::hidden;
	state.appearing_time = 0.5;
	state.disappearing_time = 0.1;
//	LOG("SimpleHaloEffect::Start end\n");

	return true;
}


//
// ����� halo
//

// ����������� � �����������
HaloEffect::HaloEffect(
					   const point3 rp,			  // ��������� �����
					   const point3& Color,
					   const HALO_EFFECT_INFO &info
					   ): BaseHalo(rp, info.Shader, info.Sound, info.SndMov)
{
	// �������� �������� �����, ������ � ���������� ���������
	ready_num = PEAK_NUM;
	// ������� �������
	//color_vector = new unsigned int [PEAK_NUM];
	//texcoord_vector = new texcoord [PEAK_NUM];
	//ready_vector = new point3 [PEAK_NUM];
	prim.Verts.resize(PEAK_NUM);
	points2d_vector = new point3 [1];
	points_vector = new point3 [1];

	far_dist = info.FarDist;
	near_dist = info.NearDist;
	large_size = info.LargeSize;
	small_size = info.SmallSize;
	near_color_factor = info.NearColorFactor;
	far_color_factor = info.FarColorFactor;
	dist_range = far_dist - near_dist;
	size_range = large_size - small_size;
	color_factor_range = far_color_factor - near_color_factor;

	color3 = Color;
	unsigned int c1 = Color.x * 255.0;
	unsigned int c2 = Color.y * 255.0;
	unsigned int c3 = Color.z * 255.0;
	color = RGB_MAKE(c1, c2, c3);
	save_color = color;
}

// ����������
HaloEffect::~HaloEffect()
{
	//delete[] color_vector;
	//delete[] texcoord_vector;
	//delete[] ready_vector;
	delete[] points2d_vector;
	delete[] points_vector;
}

// ��������� ��� �������
// (������ ������ ���������)
bool HaloEffect::NextTick(const float current_time)
{
	if(first_time) { first_time = false; Start(current_time); }
	
	// ������ � ���� halo � ����������� �� ����������
	point3 tmpcolor = color3;
	point3 camera = pGraphPipe->GetCam()->GetPos();
	float dist = (camera - root_point).Length();
	//DebugInfo::Add(100, 270, "halo: dist = %f", dist);

	if(dist < near_dist)
	{
		size = small_size;
		//DebugInfo::Add(100, 240, "halo: nearest");
	}
	else
	{
		if(dist > far_dist)
		{
			size = large_size;
			tmpcolor *= far_color_factor;
			//DebugInfo::Add(100, 240, "halo: furthest");
		}
		else
		{
			size = small_size + size_range*(dist - near_dist)/dist_range;
			tmpcolor *= (near_color_factor + color_factor_range*(dist - near_dist)/dist_range);
			//DebugInfo::Add(100, 240, "halo: middle");
		}
	}

	unsigned int c1 = tmpcolor.x * 255.0;
	unsigned int c2 = tmpcolor.y * 255.0;
	unsigned int c3 = tmpcolor.z * 255.0;

	save_color = RGB_MAKE(c1, c2, c3);
	//DebugInfo::Add(100, 300, "halo: before update visibility: save_color = %X, color = %X", save_color, color);
	UpdateVisibility(current_time);
	//DebugInfo::Add(100, 330, "halo: after update visibility: save_color = %X, color = %X", save_color, color);

	return true;
}

// � ������ ����� ������ ���������� ������
// ������� ��� ������� �������
bool HaloEffect::Start(const float start_time)
{
	// ����������� ���������� ����������
	prim.Verts[0].uv = texcoord(0, 1);
	prim.Verts[1].uv = texcoord(1, 1);
	prim.Verts[2].uv = texcoord(1, 0);
	prim.Verts[3].uv = texcoord(0, 1);
	prim.Verts[4].uv = texcoord(1, 0);
	prim.Verts[5].uv = texcoord(0, 0);
	
	prim.Verts[0].pos.z = 0;
	prim.Verts[1].pos.z = 0;
	prim.Verts[2].pos.z = 0;
	prim.Verts[3].pos.z = 0;
	prim.Verts[4].pos.z = 0;
	prim.Verts[5].pos.z = 0;

	state.prev_time = start_time;
	state.state = HaloState::hidden;
	state.appearing_time = 0.5;
	state.disappearing_time = 0.1;
	return true;
}

//
// ����� ��������� halo
//

// ����������� � �����������
BlinkingHaloEffect::BlinkingHaloEffect(
								   const point3 rp,			  // ��������� �����
								   const point3& Color,
								   const BLINKING_HALO_EFFECT_INFO &info
								   ): BaseHalo(rp, info.Shader, info.Sound, info.SndMov)
{
	// �������� �������� �����, ������ � ���������� ���������
	ready_num = PEAK_NUM;
	// ������� �������
	//color_vector = new unsigned int [PEAK_NUM];
	//texcoord_vector = new texcoord [PEAK_NUM];
	//ready_vector = new point3 [PEAK_NUM];
	prim.Verts.resize(PEAK_NUM);
	points2d_vector = new point3 [1];
	points_vector = new point3 [1];

	size = info.Size;

	unsigned int c1 = Color.x * 255.0;
	unsigned int c2 = Color.y * 255.0;
	unsigned int c3 = Color.z * 255.0;
	color = RGB_MAKE(c1, c2, c3);
	save_color = color;
	my_save_color = Color;

	sin_a = info.SinA;
}

// ����������
BlinkingHaloEffect::~BlinkingHaloEffect()
{
	//delete[] color_vector;
	//delete[] texcoord_vector;
	//delete[] ready_vector;
	delete[] points2d_vector;
	delete[] points_vector;
}

// ��������� ��� �������
// (������ ������ ���������)
bool BlinkingHaloEffect::NextTick(const float current_time)
{
	if(first_time) { first_time = false; Start(current_time); }

	UpdateVisibility(current_time);

	// �������� ��� ������
	float dt = current_time - start_time;
	const point3 ct = my_save_color*powf(sinf(sin_a*dt),2.0f)*255.0f;
	const unsigned int c1 = ct.x;
	const unsigned int c2 = ct.y;
	const unsigned int c3 = ct.z;
	save_color = RGB_MAKE(c1, c2, c3);

	return true;
}

// � ������ ����� ������ ���������� ������
// ������� ��� ������� �������
bool BlinkingHaloEffect::Start(const float start_time)
{
	// ����������� ���������� ����������
	prim.Verts[0].uv = texcoord(0, 1);
	prim.Verts[1].uv = texcoord(1, 1);
	prim.Verts[2].uv = texcoord(1, 0);
	prim.Verts[3].uv = texcoord(0, 1);
	prim.Verts[4].uv = texcoord(1, 0);
	prim.Verts[5].uv = texcoord(0, 0);
	
	prim.Verts[0].pos.z = 0;
	prim.Verts[1].pos.z = 0;
	prim.Verts[2].pos.z = 0;
	prim.Verts[3].pos.z = 0;
	prim.Verts[4].pos.z = 0;
	prim.Verts[5].pos.z = 0;

	state.prev_time = start_time;
	state.state = HaloState::hidden;
	state.appearing_time = 0.5;
	state.disappearing_time = 0.1;
	this->start_time = start_time;
	return true;
}
