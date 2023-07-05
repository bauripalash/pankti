const output = document.getElementById("output"); 
const button = document.getElementById("runbtn");
const inputBox = document.getElementById("codeinput");

const dc = new TextDecoder();

const importObject = {
	env : {
		writeStdout : (ptr , len) => {
			output.value += dc.decode(
        		new Uint8Array(wasm.memory.buffer.slice(ptr, ptr + len))
      		);

			
		},

		writeStderr : (ptr , len) => {
			output.value += dc.decode(
        		new Uint8Array(wasm.memory.buffer.slice(ptr, ptr + len))
      		);
		},

		getTimestamp : () =>{
			return Date.now();
		}
	}
}

const enc = new TextEncoder();
const makeString = (wasm , str) => {
	const strArr = enc.encode(str);
	const len = strArr.length;
	const ptr = wasm.memAlloc(len);

	if (ptr == 0) {
		output.innerText = "Failed to allocate memory";
	}

	var m8 = new Uint8Array(wasm.memory.buffer);
	for(let i = 0; i < len ; i++){
		m8[ptr + i] = strArr[i];
	}

	return {ptr , len};
}

function runCode(wasm , str){
	var sourceStr = makeString(wasm , str);
	wasm.runCodeApi(sourceStr.ptr , sourceStr.len);
	wasm.memFree(sourceStr.ptr , sourceStr.len);
}

function main(wasm){
	console.log("Module Loaded");
	button.addEventListener("click" , (_) => {
		const src = inputBox.value;
		runCode(wasm , src);
	});
}


fetch("./napi.wasm")
	.then((resp) => WebAssembly.instantiateStreaming(resp , importObject))
	.then((result) => {
		wasm = result.instance.exports;
		main(wasm);
});





