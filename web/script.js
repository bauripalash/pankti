//
// Copyright (C) Palash Bauri
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// SPDX-License-Identifier: MPL-2.0


const output = document.getElementById("output"); 
const button = document.getElementById("runbtn");
const clearButton = document.getElementById("clearbtn");
const inputBox = document.getElementById("codeinput");
const lineNumberElem = document.getElementById("line-numbers");

inputBox.value = `kaj ghumano(nayok)
show(nayok + " is sleeping!")
sesh

ghumano("Palash");
`;

const dc = new TextDecoder();



const inputBoxStyles = window.getComputedStyle(inputBox);

[
	'fontFamily',
	'fontSize',
	'fontWeight',
	'letterSpacing',
	'lineHeight',
	'padding',
].forEach((prop) => {
	lineNumberElem.style[prop] = inputBoxStyles[prop];
});

const calcNumLines = (str) => {};

const calcLineNumbers = () =>{
	const lines = inputBox.value.split('\n');
	const numLines = lines.map((line) => calcNumLines(line));

	let lineNums = [];
	let i = 1;
	while ( numLines.length > 0 ) {
		const numLinesOfSentence = numLines.shift();

		lineNums.push(i);

		if(numLinesOfSentence > 1){
			Array(numLinesOfSentence - 1)
				.fill('')
				.forEach((_) => lineNums.push(''));
		}
		i++;
	}

	return lineNums;

};

const displayLineNumbers = () =>{
	const lines = calcLineNumbers();
	lineNumberElem.innerHTML = Array.from({
		length: lines.length,
	} , (_ , i) => `<div>${lines[i] || '&nbsp'}</div>`).join("");
}

displayLineNumbers();

const ro = new ResizeObserver(() => {
	const rect = inputBox.getBoundingClientRect();
	lineNumberElem.style.height = `${rect.height}px`;
	displayLineNumbers();
});

ro.observe(inputBox);

inputBox.addEventListener('scroll' , () =>{
	lineNumberElem.scrollTop = inputBox.scrollTop;
});

inputBox.addEventListener('input' , () => {
	displayLineNumbers();
});

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
	
	clearButton.addEventListener("click" , (_) =>{
		output.value = "";
	});

	button.addEventListener("click" , (_) => {
		const src = inputBox.value;
		runCode(wasm , src);
	});
}


fetch("./pankti.wasm")
	.then((resp) => WebAssembly.instantiateStreaming(resp , importObject))
	.then((result) => {
		wasm = result.instance.exports;
		main(wasm);
});







