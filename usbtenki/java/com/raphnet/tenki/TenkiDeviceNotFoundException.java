package com.raphnet.tenki;

/**
 * Exception meaning that the requested device could not be found.
 */
public class TenkiDeviceNotFoundException extends TenkiException
{
	public TenkiDeviceNotFoundException(String serial)
	{	
		super(serial);
	}
}
