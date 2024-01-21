#!/usr/bin/nodejs

const execFile = require('child_process').execFile;

const child = execFile('usbtenkiget', ['-i','0,1,2'], (error, stdout, stderr) => {
	if (error) {
		throw error;
	}

	// Remove everything following the first newline character, then
	// split using a comma separator, then trim each field.
	var fields = stdout.replace(/(\r\n|\n|\r)*/gm,"").split(",").map(s => s.trim());

	// Validate how many fields were read
	if (fields.length != 3) {
		throw "Wrong number of fields"
	}

	// Display individual values
	console.log("Temperature.(C): " + fields[0]);
	console.log("RH..........(%): " + fields[1]);
	console.log("Pressure..(kPa): " + fields[2]);
	console.log("Temperature.(F): " + (fields[0] * 9 / 5 + 32));

});
