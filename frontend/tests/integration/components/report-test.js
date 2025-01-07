import { module, test } from 'qunit';
import { setupRenderingTest } from 'active-directory-dashboard/tests/helpers';
import { render } from '@ember/test-helpers';
import { hbs } from 'ember-cli-htmlbars';

module('Integration | Component | report', function (hooks) {
  setupRenderingTest(hooks);

  test('it renders', async function (assert) {
    // Set any properties with this.set('myProperty', 'value');
    // Handle any actions with this.set('myAction', function(val) { ... });

    await render(hbs`<Report />`);

    assert.dom().hasText('');

    // Template block usage:
    await render(hbs`
      <Report>
        template block text
      </Report>
    `);

    assert.dom().hasText('template block text');
  });
});
