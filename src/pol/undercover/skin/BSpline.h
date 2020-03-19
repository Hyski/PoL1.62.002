#pragma once


namespace PoL
{

//=====================================================================================//
//                                    class BSpline                                    //
//=====================================================================================//
template<int p>
class BSpline
{
	struct CachedPoint
	{
		float m_dist;
		point3 m_pos;

		CachedPoint() {}
		CachedPoint(float p) : m_dist(p) {}

		bool operator<(const CachedPoint &p) const
		{
			return m_dist < p.m_dist;
		}

		bool operator==(const CachedPoint &p) const
		{
			return m_dist == p.m_dist;
		}
	};

	typedef std::vector<float> Ts_t;
	typedef std::vector<point3> Ps_t;
	typedef std::vector<CachedPoint> Cps_t;

	Ts_t m_ts;
	Ps_t m_ps;

	Cps_t m_cached;
	float m_distance;

public:
	/// Добавить точку в сплайн
	void addPoint(const point3 &pt);
	/// Завершить создание сплайна
	void bake(bool fine);
	/// Очистить сплайн
	void clear();
	/// Возвращает точку на сплайне из кэша
	point3 getPoint(float pos) const;

	/// Завершен ли сплайн?
	bool isBaked() const { return !m_ts.empty(); }

	/// Получить значение базисной функции в точке pos
	float getBasis(int i, float pos) const
	{
		return getBasis<p>(i,pos);
	}

	bool isFinished(float dist) const
	{
		return dist >= m_distance;
	}

	float getDistance() const
	{
		return m_distance;
	}

private:
	/// Возвращает точку на сплайне
	point3 calcPoint(float pos) const;

	template<int pv> float getBasis(int i, float pos) const
	{
		if(pv > getn()-1) return getBasis<pv-1>(i,pos);

		const float numer1 = pos - m_ts[i];
		const float numer2 = m_ts[i+pv+1] - pos;
		const float denom1 = m_ts[i+pv] - m_ts[i];
		const float denom2 = m_ts[i+pv+1] - m_ts[i+1];

		const float factor1 = denom1 == 0.0f ? 1.0f : numer1 / denom1;
		const float factor2 = denom2 == 0.0f ? 1.0f : numer2 / denom2;

		const float basis1 = getBasis<pv-1>(i,pos);
		const float basis2 = getBasis<pv-1>(i+1,pos);

		return factor1*basis1 + factor2*basis2;
	}

	template<> float getBasis<0>(int i, float pos) const
	{
		if(m_ts[i+1] != 1.0f)
		{
			return m_ts[i] <= pos && pos < m_ts[i+1] && m_ts[i] < m_ts[i+1] ? 1.0f : 0.0f;
		}
		else
		{
			return m_ts[i] <= pos && pos <= m_ts[i+1] && m_ts[i] < m_ts[i+1] ? 1.0f : 0.0f;
		}
	}

private:
	static float dist(const point3 &p1, const point3 &p2)
	{
		const float dx = p1.x - p2.x;
		const float dy = p1.y - p2.y;
		return sqrtf(dx*dx + dy*dy);
	}

	/// Возвращает p
	int getp() const { return getn() <= p ? getn()-1 : p; }
	/// Возвращает m
	int getm() const { return getp() + getn() + 1; }
	/// Возвращает n
	int getn() const { return m_ps.size() - 1; }
};

//=====================================================================================//
//                           inline void BSpline::addPoint()                           //
//=====================================================================================//
template<int p>
inline void BSpline<p>::addPoint(const point3 &pt)
{
	assert( m_ts.empty() );
	m_ps.push_back(pt);
}

//=====================================================================================//
//                             inline void BSpline::bake()                             //
//=====================================================================================//
template<int p>
inline void BSpline<p>::bake(bool fine)
{
	assert( m_ts.empty() );

	int i = 0;
	while(getp() != p)
	{
		if(i&1)
		{
			m_ps.push_back(m_ps.back());
		}
		else
		{
			m_ps.insert(m_ps.begin(),m_ps.front());
		}
		++i;
	}

	m_ts.resize(getm()+1);

	for(int i = 0; i != m_ts.size(); ++i)
	{
		m_ts[i] = static_cast<float>(i)/static_cast<float>(getm());
	}

	for(int i = 0; i <= getp(); ++i)
	{
		m_ts[i] = 0.0f;
		m_ts[getm()-i] = 1.0f;
	}

	if(fine)
	{
		const int size = (m_ps.size()-1) * 4;
		const float inv_factor = 1.0f / static_cast<float>(size - 1);
		m_cached.resize(0);
		m_cached.reserve(size);

		m_distance = 0.0f;

		for(int i = 0; i < size; ++i)
		{
			float d = 0.0f;

			point3 pos = calcPoint(static_cast<float>(i)*inv_factor);
			if(!m_cached.empty())
			{
				d = dist(m_cached.back().m_pos,pos);
			}

			if(m_cached.empty() || d >= 1e-5f)
			{
				m_cached.push_back(CachedPoint(d+m_distance));
				m_cached.back().m_pos = pos;
				m_distance += d;
			}
		}
	}
	else
	{
		m_cached.reserve(m_ps.size());
		m_distance = 0.0f;

		const float p = 1.0f/static_cast<float>(m_ps.size()-1);
		for(int i = 0; i != m_ps.size(); ++i)
		{
			if(i)
			{
				m_distance += dist(m_ps[i],m_ps[i-1]);
			}

			m_cached.push_back(CachedPoint(m_distance));
			m_cached.back().m_pos = m_ps[i];
		}
	}
}

//=====================================================================================//
//                       inline point3 BSpline::getPoint() const                       //
//=====================================================================================//
template<int p>
inline point3 BSpline<p>::calcPoint(float pos) const
{
	assert( !m_ts.empty() );

	point3 result(0.0f,0.0f,0.0f);
	const float apos = pos;

	for(int i = 0; i <= getn(); ++i)
	{
		result += getBasis(i,apos) * m_ps[i];
	}

	return result;
}

//=====================================================================================//
//                     inline point3 BSpline<p>::getPoint() const                      //
//=====================================================================================//
template<int p>
inline point3 BSpline<p>::getPoint(float pos) const
{
	assert( !m_ts.empty() );

	pos = std::max(0.0f,std::min(m_distance,pos));

	Cps_t::const_iterator i = std::lower_bound(m_cached.begin(),m_cached.end(),CachedPoint(pos));
	Cps_t::const_iterator ip = i;
	--ip;

	if(i == m_cached.begin())
	{
		return i->m_pos;
	}
	else
	{
		return (i->m_pos - ip->m_pos) * (pos - ip->m_dist) / (i->m_dist - ip->m_dist) + ip->m_pos;
	}
}

//=====================================================================================//
//                           inline void BSpline<p>::clear()                           //
//=====================================================================================//
template<int p>
inline void BSpline<p>::clear()
{
	m_ts.clear();
	m_ps.clear();
}

typedef BSpline<1> BSpline1;
typedef BSpline<2> BSpline2;
typedef BSpline<3> BSpline3;
typedef BSpline<4> BSpline4;
typedef BSpline<5> BSpline5;
typedef BSpline<6> BSpline6;
typedef BSpline<7> BSpline7;
typedef BSpline<8> BSpline8;

}