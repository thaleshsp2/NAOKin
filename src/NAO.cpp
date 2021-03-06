#include "NAO.h"
#include <assert.h>

using namespace std;
using namespace AL;
using boost::property_tree::ptree;

typedef pair<string, string> str_str_pair;

NAO::NAO(string nao_ip) : almotion(nao_ip), alposture(nao_ip)
{
	timer.Restart();
	m_motion_filenames.push_back(pair<string, string>("standInit.json", "standInit"));
	m_motion_filenames.push_back(pair<string, string>("standZero.json", "standZero"));

	BOOST_FOREACH (str_str_pair &file_key_pair, m_motion_filenames)
	{
		ParseMotionFile(file_key_pair.first, file_key_pair.second);
	}
}

void NAO::WakeUp()
{
	almotion.wakeUp();
}

void NAO::Rest()
{
	almotion.rest();
}

void NAO::GoToPosture(string posture)
{
	alposture.goToPosture(posture, 1.0f);
}

void NAO::Trajectory()
{
	Interpolate("standInit");
	Interpolate("standZero");
}

void NAO::SetAngles(int numJoints, double speed, ...)
{
	va_list args;
	int list_length = 2 * numJoints;
	va_start(args, list_length);
	ALValue names, angles;

	for (int i = 0; i < numJoints; ++i)
	{
		names.arrayPush(va_arg(args, char *));
	}

	for (int i = numJoints; i < list_length; ++i)
	{
		angles.arrayPush(va_arg(args, double));
	}

	va_end(args);

	almotion.setAngles(names, angles, speed);
}

void NAO::ParseMotionFile(string filename, string key)
{
	ptree pt;

	try
	{
		boost::property_tree::read_json(filename, pt);
	}
	catch (boost::exception &ex)
	{
		cout << "File " << filename << " could not be loaded." << endl;
	}

	InterpolationValues ip_values;

	BOOST_FOREACH (const ptree::value_type &element, pt.get_child("root"))
	{
		vector<float> angles_vector;
		vector<float> times_vector;

		BOOST_FOREACH (const ptree::value_type &ele, element.second.get_child("angles"))
		{
			angles_vector.push_back(ele.second.get<float>(""));
		}
		BOOST_FOREACH (const ptree::value_type &ele, element.second.get_child("times"))
		{
			times_vector.push_back(ele.second.get<float>(""));
		}

		ip_values.names.push_back(element.second.get<string>("name"));
		ip_values.angles.push_back(angles_vector);
		ip_values.times.push_back(times_vector);
	}
	m_interpolations[key] = ip_values;
}

void NAO::Interpolate(string key)
{
	InterpolationValues iv = m_interpolations[key];

	ALValue al_names,
			al_angles,
			al_times;

	unsigned size = iv.angles.size();
	for (unsigned i = 0; i < size; ++i)
	{
		al_names.arrayPush(iv.names[i]);
		al_angles.arrayPush(iv.angles[i]);
		al_times.arrayPush(iv.times[i]);
	}
	//Naoqi interpolation
	almotion.angleInterpolation(iv.names, al_angles, al_times, true);
}

void NAO::Stiff()
{
	almotion.setStiffnesses("Body", 0.8);
}

void NAO::Unstiff()
{
	almotion.setStiffnesses("Body", 0);
}