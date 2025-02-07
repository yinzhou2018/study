async function async_fn_3() {
  console.log('produasync_fn_2');
  return 10;
}

async function async_fn_2() {
  console.log('produasync_fn_2');
  const value = await async_fn_3();
  console.log(`value: ${value}`);
  return 10;
}

async function async_fn_1() {
  console.log('async_fn_1');
  const value = await async_fn_2();
  console.log(`value: ${value}`);
}

async function main() {
  await async_fn_1();
}

main();
console.log('finsihed.');
