/**
 * @brief VFR HUD plugin
 * @file vfr_hud.cpp
 * @author Vladimir Ermakov <vooon341@gmail.com>
 *
 * @addtogroup plugin
 * @{
 */
/*
 * Copyright 2014,2016 Vladimir Ermakov.
 *
 * This file is part of the mavros package and subject to the license terms
 * in the top-level LICENSE file of the mavros repository.
 * https://github.com/mavlink/mavros/tree/master/LICENSE.md
 */

#include <angles/angles.h>
#include <mavros/mavros_plugin.h>

#include <mavros_msgs/VFR_HUD.h>
#include <geometry_msgs/TwistStamped.h>

namespace mavros {
namespace std_plugins {
/**
 * @brief VFR HUD plugin.
 */
class VfrHudPlugin : public plugin::PluginBase {
public:
	VfrHudPlugin() : PluginBase(),
		nh("~")
	{ }

	/**
	 * Plugin initializer. Constructor should not do this.
	 */
	void initialize(UAS &uas_)
	{
		PluginBase::initialize(uas_);

		vfr_pub = nh.advertise<mavros_msgs::VFR_HUD>("vfr_hud", 10);
		wind_pub = nh.advertise<geometry_msgs::TwistStamped>("wind_estimation", 10);
	}

	Subscriptions get_subscriptions()
	{
		return {
			make_handler(&VfrHudPlugin::handle_vfr_hud),
			make_handler(&VfrHudPlugin::handle_wind),
		};
	}

private:
	ros::NodeHandle nh;

	ros::Publisher vfr_pub;
	ros::Publisher wind_pub;

	void handle_vfr_hud(const mavlink::mavlink_message_t *msg, mavlink::common::msg::VFR_HUD &vfr_hud)
	{
		auto vmsg = boost::make_shared<mavros_msgs::VFR_HUD>();
		vmsg->header.stamp = ros::Time::now();
		vmsg->airspeed = vfr_hud.airspeed;
		vmsg->groundspeed = vfr_hud.groundspeed;
		vmsg->heading = vfr_hud.heading;
		vmsg->throttle = vfr_hud.throttle / 100.0;	// comes in 0..100 range
		vmsg->altitude = vfr_hud.alt;
		vmsg->climb = vfr_hud.climb;

		vfr_pub.publish(vmsg);
	}

	/**
	 * Handle APM specific wind direction estimation message
	 */
	void handle_wind(const mavlink::mavlink_message_t *msg, mavlink::ardupilotmega::msg::WIND &wind)
	{
		const double speed = wind.speed;
		const double course = angles::from_degrees(wind.direction);

		auto twist = boost::make_shared<geometry_msgs::TwistStamped>();
		twist->header.stamp = ros::Time::now();
		// TODO: check math's
		twist->twist.linear.x = speed * std::sin(course);
		twist->twist.linear.y = speed * std::cos(course);
		twist->twist.linear.z = wind.speed_z;

		wind_pub.publish(twist);
	}
};
}	// namespace std_plugins
}	// namespace mavros

#include <pluginlib/class_list_macros.h>
PLUGINLIB_EXPORT_CLASS(mavros::std_plugins::VfrHudPlugin, mavros::plugin::PluginBase)
