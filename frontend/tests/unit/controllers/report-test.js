import { module, test } from 'qunit';
import { setupTest } from 'active-directory-dashboard/tests/helpers';

module('Unit | Controller | report', function (hooks) {
  setupTest(hooks);

  // TODO: Replace this with your real tests.
  test('it exists', function (assert) {
    let controller = this.owner.lookup('controller:report');
    assert.ok(controller);
  });
});
