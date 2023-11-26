async function hello1() {
  console.log("hello1");
  return 10;
}

async function hello2() {
  console.log("hello2");
  return hello1();
}

async function main() {
  value = await hello2();
  console.log(value);
}

main();
console.log("finsihed.");