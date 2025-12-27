FaustGenDef {
	classvar <all;
	var <name;
	var <hash;
	var <>code;
	var <>params;

	// private
	classvar counter;
	// allows to obtain an object from a given hash
	// which is necessary for callbacks from the server
	classvar <>prHashMap;
	classvar recvFunc;

	*initClass {
		all = IdentityDictionary();
		prHashMap = IdentityDictionary();
		counter = 0;

		// this function receives the parsed parameters from the server
		// since it is difficult to send string data from the server
		// to the client, the content is shared via a file.
		// The client creates a file and sends its path to the server,
		// and the server will write the parsed parameters into this
		// file. After the server has written the content, the language
		// will be notified via a doneMsg, which will trigger this
		// function.
		recvFunc = {|msg, time, replyAddr, recvPort|
			if(msg.size == 2, {
				var doneMsg = msg[1].asString;
				if(doneMsg.beginsWith("faustgen"), {
					var hash = doneMsg.replace("faustgen", "").asInteger;
					var res = FaustGenDef.prHashMap[hash];
					var paramPath = FaustGenDef.prParamPath(hash);
					var paramString = File.readAllString(paramPath);
					var params = paramString.split($$);
					// remove last empty param
					params = params.keep(params.size-1);
					// convert to symbol for lookup
					res.params = params.collect({|p| p.asSymbol});

					"Set faust params for %".format(res).postln;
					File.delete(paramPath);
				});
			});
		};
		thisProcess.addOSCRecvFunc(recvFunc);
	}

	*new {|name, code|
		var res;
		var hash;

		name = name.asSymbol;
		res = all[name];
		if(res.notNil, {
			if(code.isNil.not, {
				res.code = code;
			});
			^res;
		});
		hash = FaustGenDef.prHashSymbol(name);
		res = super.newCopyArgs(name, hash, code ? "");
		all[name] = res;
		FaustGenDef.prHashMap[hash] = res;
		^res;
	}

	*load {|name, path|
		var code = File.readAllString(path.asAbsolutePath);
		^this.new(name, code);
	}

	load {|path|
		code = File.readAllString(path.asAbsolutePath);
	}

	send {|server, completionMsg|
		var servers = (server ?? { Server.allBootedServers }).asArray;
		servers.do { |each|
			if(each.hasBooted.not) {
				"Server % not running, could not send FaustGenDef.".format(server.name).warn
			};
			this.prSendScript(each, completionMsg);
		}
	}

	prSendScript {|server, completionMsg|
		var tempPath = FaustGenDef.prParamPath(hash);
		var message = [\cmd, \faustgenscript, hash, tempPath, code, completionMsg].asRawOSC;
		if(message.size < (65535 div: 4), {
			server.sendRaw(message);
		}, {
			this.prSendFile(server, completionMsg);
		});
	}

	prSendFile {|server, completionMsg|
		var tmpFilePath = PathName.tmp +/+ "%_%".format(hash.asString, counter);
		counter = counter + 1;

		if (server.isLocal.not, {
			"FaustGenDef % could not be added  to server % because it is too big for sending via OSC and server is not local".format(
				name,
				server,
			).warn;
			^this;
		});

		File.use(tmpFilePath, "w", {|f|
			f.write(code);
		});

		server.sendMsg(\cmd, \faustgenfile, hash, tmpFilePath, completionMsg);

		fork {
			var deleteSuccess;
			server.sync;
			deleteSuccess = File.delete(tmpFilePath);
			if (deleteSuccess.not, {
				"Could not delete temp file % of FaustGenDef %".format(tmpFilePath, name).warn;
			});
		}
	}

	// converts a list of [p1k, p1v, p2k, p2v] to
	// a list of [p1pos, p1v, p2pos, p2v]
	prParamMap {|inputs|
		var map = [];
		inputs.pairsDo({|k, v|
			var pos = params.indexOf(k);
			if(pos.notNil, {
				map = map.add(pos).add(v);
			}, {
				"Could not find param % for %".format(k, this.name).warn;
			});
		});
		^map.postln;
	}

	*prHashSymbol {|symbol|
		// hash numbers are too high to represent as a float32
		// on the server, so we have to scale those down.
		// 2**20 seems okayish?
		^(symbol.hash.abs % (2**20) * symbol.hash.sign).asInteger;
	}

	*prParamPath {|hash|
		^Platform.defaultTempDir +/+ "faustgen%".format(hash);
	}
}


FaustGen : MultiOutUGen {
	*ar {|numOutputs, script, inputs, parameters|
		var parameterMap;
		var combinedInputs;

		script = case
		{script.isKindOf(FaustGenDef)} {script}
		{script.isKindOf(String)} {FaustGenDef.all[script.asSymbol]}
		{script.isKindOf(Symbol)} {FaustGenDef.all[script.asSymbol]}
		{Error("Script input needs to be a FaustGenDef object or a symbol, found %".format(script.class)).throw};
		if(script.isNil, {
			Error("Could not find script").throw;
		});

		parameterMap = script.prParamMap(parameters);

		combinedInputs = inputs ++ parameterMap;

		^this.multiNew('audio', numOutputs, FaustGenDef.prHashSymbol(script.name).asFloat, inputs.size, parameterMap.size/2, *combinedInputs);
	}

	init { |numOutputs ... theInputs|
		inputs = theInputs;
		inputs = inputs.asArray.collect({|input|
			if(input.rate != \audio, {
				K2A.ar(input);
			}, {
				input;
			});
		});
		^this.initOutputs(numOutputs, 'audio');
	}
}
