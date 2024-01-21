import java.io.*;

public class JavaUsbTenkiGet
{
	public static void main(String[] args)
	{
		Process usbtenkiget;

		// Note: usbtenkiget assumed to be in the path
		// Arguments passed to -i (0,1,2) here need to be updated to fit
		// your scenario. You may also specify a serial number by adding
		// the -s argument.
		try {
			usbtenkiget = Runtime.getRuntime().exec("usbtenkiget -i 0,1,2");
		} catch(IOException e) {
			System.err.println("could not run usbtenkiget: " + e);
			return;
		}

		// Wait until the process exits
		while(true) {
			try {
				usbtenkiget.waitFor();
			} catch (InterruptedException e) {
				continue;
			}
			break;
		}

		// Check if usbtenkiget exited with an error code
		if (usbtenkiget.exitValue() != 0) {
			System.err.println("usbtenkiget error. Exit value=" + usbtenkiget.exitValue());
			return;
		}

		BufferedReader reader = new BufferedReader(new InputStreamReader(usbtenkiget.getInputStream()));

		// usbtenkiget outputs the data on the first line. Read it to a string.
		String line;
		try {
			line = reader.readLine();
		} catch (IOException e) {
			System.err.println("Error reading data: " + e);
			return;
		}

		// Now split the line in an array of values.
		String[] values = line.split(",");

		// Check that we received the expected number of fields (in this case,
		// the usbtenkiget -i 0,1,2 argument requests 3 fields).
		if (values.length != 3) {
			System.err.println("Incorrect number of fields received: " + values.length);
			return;
		}

		float temperature = Float.parseFloat(values[0]);
		float rh = Float.parseFloat(values[1]);
		float pressure = Float.parseFloat(values[2]);

		System.out.println("Temperature (C):" + temperature);
		System.out.println("RH......... (%):" + rh);
		System.out.println("Pressure..(kPa):" + pressure);
		System.out.println("Temperature (C):" + (temperature*9/5+32));
	}
}
