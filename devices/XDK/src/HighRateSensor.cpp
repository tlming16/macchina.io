//
// HighRateSensor.cpp
//
// $Id$
//
// Copyright (c) 2017, Applied Informatics Software Engineering GmbH.
// All rights reserved.
//
// SPDX-License-Identifier: Apache-2.0
//


#include "HighRateSensor.h"
#include "Poco/NumberFormatter.h"
#include "Poco/Delegate.h"


namespace IoT {
namespace BtLE {
namespace XDK {


const std::string HighRateSensor::NAME("XDK Sensor");
const std::string HighRateSensor::SYMBOLIC_NAME("io.macchina.btle.xdk");


HighRateSensor::HighRateSensor(Peripheral::Ptr pPeripheral, const Params& params):
	_params(params),
	_pPeripheral(pPeripheral),
	_ready(false),
	_enabled(false),
	_value(0),
	_valueChangedDelta(0.0),
	_pEventPolicy(new IoT::Devices::NoModerationPolicy<double>(valueChanged)),
	_deviceIdentifier(pPeripheral->address()),
	_symbolicName(SYMBOLIC_NAME),
	_name(NAME),
	_physicalQuantity(params.physicalQuantity),
	_physicalUnit(params.physicalUnit)
{
	addProperty("displayValue", &HighRateSensor::getDisplayValue);
	addProperty("enabled", &HighRateSensor::getEnabled, &HighRateSensor::setEnabled);
	addProperty("connected", &HighRateSensor::getConnected);
	addProperty("valueChangedDelta", &HighRateSensor::getValueChangedDelta, &HighRateSensor::setValueChangedDelta);
	addProperty("deviceIdentifier", &HighRateSensor::getDeviceIdentifier);
	addProperty("symbolicName", &HighRateSensor::getSymbolicName);
	addProperty("name", &HighRateSensor::getName);
	addProperty("physicalQuantity", &HighRateSensor::getPhysicalQuantity);
	addProperty("physicalUnit", &HighRateSensor::getPhysicalUnit);
	
	_pEventPolicy = new IoT::Devices::NoModerationPolicy<double>(valueChanged);

	_pPeripheral->connected += Poco::delegate(this, &HighRateSensor::onConnected);
	_pPeripheral->disconnected += Poco::delegate(this, &HighRateSensor::onDisconnected);

	init();
}

	
HighRateSensor::~HighRateSensor()
{
	_pPeripheral->connected -= Poco::delegate(this, &HighRateSensor::onConnected);
	_pPeripheral->disconnected -= Poco::delegate(this, &HighRateSensor::onDisconnected);

	_pPeripheral = 0;
}


bool HighRateSensor::isConnected() const
{
	return _pPeripheral->isConnected();
}


double HighRateSensor::value() const
{
	Poco::Mutex::ScopedLock lock(_mutex);

	return _value;
}


bool HighRateSensor::ready() const
{
	Poco::Mutex::ScopedLock lock(_mutex);

	return _ready && _pPeripheral->isConnected();	
}


void HighRateSensor::enable(bool enabled)
{
	Poco::Mutex::ScopedLock lock(_mutex);

	_enabled = enabled;
}


Poco::Any HighRateSensor::getEnabled(const std::string&) const
{
	Poco::Mutex::ScopedLock lock(_mutex);

	return _enabled;
}


void HighRateSensor::setEnabled(const std::string&, const Poco::Any& value)
{
	enable(Poco::AnyCast<bool>(value));
}


Poco::Any HighRateSensor::getConnected(const std::string&) const
{
	return isConnected();
}


Poco::Any HighRateSensor::getValueChangedDelta(const std::string&) const
{
	Poco::Mutex::ScopedLock lock(_mutex);

	return _valueChangedDelta;
}


void HighRateSensor::setValueChangedDelta(const std::string&, const Poco::Any& value)
{
	Poco::Mutex::ScopedLock lock(_mutex);

	double delta = Poco::AnyCast<double>(value);
	if (delta != _valueChangedDelta)
	{
		if (delta == 0)
		{
			_pEventPolicy = new IoT::Devices::NoModerationPolicy<double>(valueChanged);
		}
		else
		{
			_pEventPolicy = new IoT::Devices::MinimumDeltaModerationPolicy<double>(valueChanged, _value, delta);
		}
		_valueChangedDelta = delta;
	}
}


Poco::Any HighRateSensor::getDisplayValue(const std::string&) const
{
	if (_ready)
		return Poco::NumberFormatter::format(value(), 0, 1);
	else
		return std::string("n/a");
}


Poco::Any HighRateSensor::getDeviceIdentifier(const std::string&) const
{
	return _deviceIdentifier;
}


Poco::Any HighRateSensor::getName(const std::string&) const
{
	return _name;
}


Poco::Any HighRateSensor::getSymbolicName(const std::string&) const
{
	return _symbolicName;
}


Poco::Any HighRateSensor::getPhysicalQuantity(const std::string&) const
{
	return _physicalQuantity;
}


Poco::Any HighRateSensor::getPhysicalUnit(const std::string&) const
{
	return _physicalUnit;
}


void HighRateSensor::update(double value)
{
	Poco::Mutex::ScopedLock lock(_mutex);

	if (!_ready || _value != value)
	{
		_ready = true;
		_value = value;
		_pEventPolicy->valueChanged(value);
	}
}


void HighRateSensor::init()
{
	enable(true);
}


void HighRateSensor::onConnected()
{
	if (_enabled)
	{
		enable(true);
	}
}


void HighRateSensor::onDisconnected()
{
	enable(false);
}


} } } // namespace IoT::BtLE::XDK
