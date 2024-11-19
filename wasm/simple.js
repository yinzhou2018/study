fetch("http://localhost/simple.wasm").then(response => {
   response.arrayBuffer().then(buffer => {
      const module = new WebAssembly.Module(buffer);
      const instance = new WebAssembly.Instance(module);
      const result = instance.exports.call_add();
      console.log(result);
   });
})