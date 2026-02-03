export function isTrionic(nums: number[]): boolean {
  const n = nums.length;
  if (n < 3) {
    return false;
  }

  let p = 0;

  // Phase 1: strictly increasing
  while (p < n - 2 && nums[p] < nums[p + 1]) {
    p++;
  }
  if (p === 0) {
    return false; // must have at least one increasing step
  }

  // Phase 2: strictly decreasing
  let q = p;
  while (q < n - 1 && nums[q] > nums[q + 1]) {
    q++;
  }
  if (q === p || q === n - 1) {
    return false; // must decrease at least once and leave room for phase 3
  }

  // Phase 3: strictly increasing
  while (q < n - 1 && nums[q] < nums[q + 1]) {
    q++;
  }

  return q === n - 1;
}
