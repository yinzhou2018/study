
async function produce() {
  console.log("produce");
  return 10;
}

async function consume() {
  const value = await produce();
  console.log("consume", value);
  return value;
}

async function main() {
  console.log(value);
}

function* gen(){
  yield 1;
  yield 2;
  return 3;
} 


main();
console.log("finsihed.");