/*
 * Copyright 2006 Phidgets Inc.  All rights reserved.
 */

package com.phidgets.event;

import com.phidgets.Phidget;

/**
 * This class represents the data for a ServerConnectEvent.
 * 
 * @author Phidgets Inc.
 */
public class ServerConnectEvent
{
	Object source;

	/**
	 * Class constructor. This is called internally by the phidget library when creating this event.
	 * 
	 * @param source the object from which this event originated
	 */
	public ServerConnectEvent(Object source)
	{
		this.source = source;
	}

	/**
	 * Returns the source Object of this event. This is a reference to the object from which this
	 * event was called. This object can be cast into a specific type of Phidget object to call specific
	 * device calls on it.
	 * 
	 * @return the event caller
	 */
	public Object getSource()
	{
		return source;
	}

	/**
	 * Returns a string containing information about the event.
	 * 
	 * @return an informative event string
	 */
	public String toString() {
		return source.toString();
	}
}
