// orbit.h
//
// Copyright (C) 2001, Chris Laurel <claurel@shatters.net>
//
// CometOrbit: Copyright (C) 2007,2008 Johannes Gajdosik
//             Amendments (c) 2013 Georg Zotti
//             Recombined to KeplerOrbit (c) 2019 Georg Zotti
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

#ifndef ORBIT_HPP
#define ORBIT_HPP

#include "VecMath.hpp"
#include "StelUtils.hpp"

//! @internal
//! Orbit computations used for comets, minor planets and "simple" moons
class Orbit
{
public:
    Orbit(void) {}
    virtual ~Orbit(void) {}
private:
    Orbit(const Orbit&);
    const Orbit &operator=(const Orbit&);
};

/*
class EllipticalOrbit : public Orbit
{
public:
	EllipticalOrbit(double pericenterDistance,
			double eccentricity,
			double inclination,
			double ascendingNode,
			double argOfPeriapsis,
			double meanAnomalyAtEpoch,
			double period,
			double epoch,			// = 2451545.0,
			double parentRotObliquity,	// = 0.0,
			double parentRotAscendingnode,	// = 0.0
			double parentRotJ2000Longitude	// = 0.0
			);

	// Compute position for a specified Julian date and return coordinates
	// given in "dynamical equinox and ecliptic J2000"
	// which is the reference frame for VSOP87
	// In order to rotate to VSOP87
	// parentRotObliquity and parentRotAscendingnode must be supplied.
	void positionAtTimevInVSOP87Coordinates(const double JDE, double* v) const;

	// Original one
	Vec3d positionAtTime(const double JDE) const;
	double getPeriod() const{return period;}
private:
	//! returns eccentric anomaly E for Mean anomaly M
	double eccentricAnomaly(const double M) const;
	Vec3d positionAtE(const double E) const;

	double pericenterDistance;
	double eccentricity;
	double inclination;
	double ascendingNode;
	double argOfPeriapsis;
	double meanAnomalyAtEpoch;
	double period;
	double epoch;
	double rotateToVsop87[9];
};
*/

// This class was called CometOrbit, but was now recombined with the former EllipticalOrbit class. They did almost the same.
class KeplerOrbit : public Orbit {
public:
	KeplerOrbit(double pericenterDistance,
		   double eccentricity,
		   double inclination,
		   double ascendingNode,
		   double argOfPerhelion,
		   double timeAtPerihelion,
		   double orbitGoodDays,
		   double meanMotion,			// GZ: for parabolics, this is W/dt in Heafner's lettering
		   double parentRotObliquity,		// Comets only have parent==sun, no need for these? Oh yes, VSOP/J2000 eq frames!
		   double parentRotAscendingnode,
		   double parentRotJ2000Longitude
		   );
	//! Compute the orbit for a specified Julian day and return a "stellarium compliant" function
	void positionAtTimevInVSOP87Coordinates(double JDE, double* v);
	//! updating comet tails is a bit expensive. try not to overdo it.
	bool getUpdateTails() const { return updateTails; }
	void setUpdateTails(const bool update){ updateTails=update; }
	//! return speed value [AU/d] last computed by positionAtTimevInVSOP87Coordinates(JDE, v)
	Vec3d getVelocity() const { return rdot; }
	void getVelocity(double *vel) const { vel[0]=rdot[0]; vel[1]=rdot[1]; vel[2]=rdot[2];}
	//! Returns semimajor axis [AU] for elliptic orbit, 0 for a parabolic orbit, and a negative value for hyperbolic orbit.
	double getSemimajorAxis() const { return (e==1. ? 0. : q / (1.-e)); }
	double getEccentricity() const { return e; }
	//! For planet moons which have orbits given in relation to their parent planet's equator.
	//! This is called by the constructor, and must be updated for parent planets when their axis changes over time.
	void setParentOrientation(const double parentRotObliquity, const double parentRotAscendingNode, const double parentRotJ2000Longitude);
	bool objectDateValid(const double JDE) const { return ((orbitGood<=0) || (fabs(t0-JDE)<orbitGood)); }
	//! Calculate sidereal period in days from semi-major axis. If sMA<0 (hyperbolic orbit), return max double.
	//! NOTE: The result is for a solar-centered orbit only!
	static double calculateSiderealPeriod(const double semiMajorAxis);

private:
	const double q;  //! perihel distance
	const double e;  //! eccentricity
	const double i;  //! inclination
	const double Om; //! longitude of ascending node
	const double w;  //! argument of perihel
	const double t0; //! time of perihel, JDE
	const double n;  //! mean motion (for parabolic orbits: W/dt in Heafner's presentation)
	Vec3d rdot;      //! GZ: velocity vector. Caches velocity from last position computation, [AU/d]
	double rotateToVsop87[9]; //! Rotation matrix
	bool updateTails; //! flag to signal that tails must be recomputed.
	const double orbitGood; //! orb. elements are only valid for this time from perihel [days]. Don't draw the object outside. Values <=0 mean "always good" (objects on elliptic orbit)
};

//! A pseudo-orbit for "observers" linked to a planet's sphere. It allows setting distance and longitude/latitude in the VSOP87 frame.
class GimbalOrbit : public Orbit {
public:
	GimbalOrbit(double distance,
		   double longitude,
		   double latitude
		   );
	//! Compute the orbit for a specified Julian day and return a "stellarium compliant" function
	void positionAtTimevInVSOP87Coordinates(double JDE, double* v);
	//! return speed value [AU/d] last computed by positionAtTimevInVSOP87Coordinates(JDE, v)
	Vec3d getVelocity() const { return Vec3d(0.); }
	void getVelocity(double *vel) const { vel[0]=0.; vel[1]=0.; vel[2]=0.;}
	double getSemimajorAxis() const { return distance; } //! Returns semimajor axis [AU] for elliptic orbit, 0 for a parabolic orbit, and a negative value for hyperbolic orbit.
	double getEccentricity() const { return 0; }
	//! For planet moons which have orbits given in relation to their parent planet's equator.
	//! This is called by the constructor, and must be updated for parent planets when their axis changes over time.
	void setParentOrientation(const double parentRotObliquity, const double parentRotAscendingNode, const double parentRotJ2000Longitude);

	double getLongitude() const { return longitude*M_180_PI;}
	double getLatitude()  const { return latitude*M_180_PI;}
	double getDistance()  const { return distance;}
	void setLongitude(const double lng){ longitude=lng*M_PI_180;}
	void setLatitude(const double lat) { latitude=lat*M_PI_180;}
	void setDistance(const double dist){ distance=dist;}
	void addToLongitude(const double dlong){ longitude+=dlong*M_PI_180; }
	void addToLatitude(const double dlat)  { latitude=qBound(-M_PI_2, latitude+dlat*M_PI_180, M_PI_2);}
	void addToDistance(const double ddist) { distance=qBound(0.01, distance+ddist, 50.);}

private:
	double distance;   //! distance to parent planet center, AU
	double longitude;  //! longitude [radians]
	double latitude;   //! latitude [radians]
	double rotateToVsop87[9]; //! Rotation matrix.
};
#endif // ORBIT_HPP
