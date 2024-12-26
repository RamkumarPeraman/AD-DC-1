import { module, test } from 'qunit';
import { setupTest } from 'active-directory-dashboard/tests/helpers';

module('Unit | Controller | deleted-objects', function (hooks) {
  setupTest(hooks);

  // TODO: Replace this with your real tests.
  test('it exists', function (assert) {
    let controller = this.owner.lookup('controller:deleted-objects');
    assert.ok(controller);
  });
});
