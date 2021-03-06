#include <rtt/Component.hpp>

#include "irp6p_transmission.h"

const double GEAR[6] = {-158.0, 2*M_PI/5.0, 2*M_PI/5.0, -128.0, -128.0*0.6, 288.8845};
const double SYNCHRO_MOTOR_POSITION[6] = {-15.9, -5.0, -8.527, 151.31, 432.25, 791.0};
const double THETA[6] = {0.0, 2.203374e+02, 1.838348e+02, 1.570796e+00, 0.0, 0.0};

const double 	SYNCHRO_JOINT_POSITION[6] = { SYNCHRO_MOTOR_POSITION[0] - GEAR[0] * THETA[0],
                                            SYNCHRO_MOTOR_POSITION[1] - GEAR[1] * THETA[1],
	                                          SYNCHRO_MOTOR_POSITION[2] - GEAR[2] * THETA[2],
	                                          SYNCHRO_MOTOR_POSITION[3] - GEAR[3] * THETA[3],
	                                          SYNCHRO_MOTOR_POSITION[4] - GEAR[4] * THETA[4] - SYNCHRO_MOTOR_POSITION[3],
	                                          SYNCHRO_MOTOR_POSITION[5] - GEAR[5] * THETA[5] };

const int ENC_RES[6] = {4000, 4000, 4000, 4000, 4000, 2000};

const double LOWER_MOTOR_LIMIT[6] = { -470, -110, -80, -70, -80, -1000};
const double UPPER_MOTOR_LIMIT[6] = { 450, 100, 100, 380, 490, 3000};

IRP6PTransmission::IRP6PTransmission(const std::string& name) : RTT::TaskContext(name, PreOperational) {

	this->ports()->addPort("MotorPosition", port_motor_position_);
	this->ports()->addPort("JointPosition", port_joint_position_);
}

IRP6PTransmission::~IRP6PTransmission() {

}

bool IRP6PTransmission::configureHook() {
	motor_position_.resize(6);
	joint_position_.resize(6);
	return true;
}

void IRP6PTransmission::updateHook() {
	port_motor_position_.read(motor_position_);
	mp2i(&motor_position_(0), &joint_position_(0));	
	port_joint_position_.write(joint_position_);
}

void IRP6PTransmission::mp2i(const double* motors, double* joints)
{
  // zmienne pomocnicze
  double c, d, l;
  double sinus, cosinus;
  double M1, M2;

  const double sl123 = 7.789525e+04;
  const double mi1 = 6.090255e+04;
  const double ni1 = -2.934668e+04;

  const double mi2 = -4.410000e+04;
  const double ni2 = -5.124000e+04;

// Przelicznik polozenia walu silnika napedowego kolumny w radianach
// na kat obrotu kolumny (wspolrzedna wewnetrzna) w radianach
  joints[0] = (motors[0] - SYNCHRO_MOTOR_POSITION[0]) / GEAR[0] + THETA[0];

// Przelicznik polozenia walu silnika napedowego ramienia dolnego w radianach
// na kat obrotu ramienia (wspolrzedna wewnetrzna) w radianach
  l = (motors[1] - SYNCHRO_MOTOR_POSITION[1]) / GEAR[1] + THETA[1];
  M1 = mi1 * mi1 + ni1 * ni1;
  c = l * l - sl123;
  d = sqrt(M1 - c * c);
  cosinus = (mi1 * c - ni1 * d) / M1;
  sinus = -(ni1 * c + mi1 * d) / M1;
  joints[1] = atan2(sinus, cosinus);

// Przelicznik polozenia walu silnika napedowego ramienia gornego w radianach
// na kat obrotu ramienia (wspolrzedna wewnetrzna) w radianach
  l = (motors[2] - SYNCHRO_MOTOR_POSITION[2]) / GEAR[2] + THETA[2];
  M2 = mi2 * mi2 + ni2 * ni2;
  c = l * l - sl123;
  d = sqrt(M2 - c * c);
  cosinus = (mi2 * c - ni2 * d) / M2;
  sinus = -(ni2 * c + mi2 * d) / M2;
  joints[2] = atan2(sinus, cosinus);

// Przelicznik polozenia walu silnika napedowego obrotu kisci T w radianach
// na kat pochylenia kisci (wspolrzedna wewnetrzna) w radianach
  joints[3] = (motors[3] - SYNCHRO_MOTOR_POSITION[3]) / GEAR[3];

// Przelicznik polozenia walu silnika napedowego obrotu kisci V w radianach
// na kat obrotu kisci (wspolrzedna wewnetrzna) w radianach
  joints[4] = (motors[4] - SYNCHRO_MOTOR_POSITION[4] - (motors[3]
                             - SYNCHRO_MOTOR_POSITION[3])) / GEAR[4] + THETA[4];

// Przelicznik polozenia walu silnika napedowego obrotu kisci N w radianach
// na kat obrotu kisci (wspolrzedna wewnetrzna) w radianach
  joints[5] = (motors[5] - SYNCHRO_MOTOR_POSITION[5]) / GEAR[5] + THETA[5];

  joints[2] -= joints[1] + M_PI_2;
  joints[3] -= joints[2] + joints[1] + M_PI_2;

}

ORO_CREATE_COMPONENT(IRP6PTransmission)

