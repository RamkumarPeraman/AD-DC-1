import { helper } from '@ember/component/helper';

export default helper(function eq([leftSide, rightSide]) {
  return leftSide === rightSide;
});
