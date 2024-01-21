package com.raphnet.tenki;

/**
 * Direct representation of a device's channel.
 */
public class TenkiDeviceChannel
{
	private int _id;
	private String _name;
	private String _type_name;
	private int _chip_id;
	private TenkiChannelDataSource _dev;
	
	private boolean data_valid = false;

	/**
	 * Constructor.
	 *
	 * This creates a TenkiDeviceChannel which will
	 * use the specified data source.
	 *
	 * @param dev Data source for channel
	 * @param id Channel id
	 * @param name The name of the channel
	 * @param chip_id The chip id of the channel
	 * @param type_name The type of channel
	 */
	public TenkiDeviceChannel(TenkiChannelDataSource dev, int id, String name, int chip_id, String type_name)
	{
		_dev = dev;
		_id = id;
		_name = name;
		_chip_id = chip_id;
		_type_name = type_name;
	}

	/** 
	 * Force a (re-)read of this channel value from the device.
	 *
	 * @throws TenkiException if something goes wrong.
	 **/
	public void refresh() throws TenkiException
	{
		_dev.readChannel(this);
	}

	/**
	 * Get the most recently read value of this channel.
	 *
	 * {@link #refresh()} will be called automatically if
	 * this channel was never read. Afterwards, it is your
	 * responsability to call refresh() to update the cache. Note
	 * that the cache will also be updated when using uncached methods
	 * such as {@link #getValue()}.
	 *
	 * @throws TenkiException if something goes wrong.
	 * @return The channel value 
	 */
	public float getValueCached() throws TenkiException
	{ 
		if (!data_valid)
			refresh();
		return _dev.getConvertedChannelValue(this);
	}

	/**
	 * Get the current value for this channel from the device.
	 *
	 * This updates the cache as well.
	 *
	 * @throws TenkiException if something goes wrong.
	 * @return The channel value
	 */
	public float getValue() throws TenkiException
	{
		refresh();
		return getValueCached();
	}

	/**
	 * Get the unit in which this channel data's is represented.
	 *
	 * @see com.raphnet.tenki.TenkiConstants
	 * @throws TenkiException if something goes wrong.
	 * @return The channel unit.
	 */
	public int getUnit() throws TenkiException
	{
		if (!data_valid)
			refresh();	
		return _dev.getConvertedChannelUnit(this);
	}

	/**
	 * Get this channel's Id.
	 *
	 * The channel id is used by the TenkiChannelDataSource. The scope
	 * of this ID is valid only within the assotiated TenkiChannelDataSource.
	 *
	 * @return The channel id.
	 */
	public int getId() { return _id; }

	/**
	 * Get the name of this channel.
	 *
	 * @return The channel name.
	 */
	public String getName() { return _name; }

	/** 
	 * Get the chip id of this channel.
	 * @see com.raphnet.tenki.TenkiConstants
	 * @return The chip id.
	 * */
	public int getChipId() { return _chip_id; }

	/**
	 * Get this channel's type name.
	 * @return The type name of the channel.
	 */
	public String getTypeName() { return _type_name; }
}


