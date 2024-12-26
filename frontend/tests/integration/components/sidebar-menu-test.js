import { module, test } from 'qunit';
import { setupRenderingTest } from 'active-directory-dashboard/tests/helpers';
import { render } from '@ember/test-helpers';
import { hbs } from 'ember-cli-htmlbars';

module('Integration | Component | sidebar-menu', function (hooks) {
  setupRenderingTest(hooks);

  test('it renders', async function (assert) {
    // Set any properties with this.set('myProperty', 'value');
    // Handle any actions with this.set('myAction', function(val) { ... });

    await render(hbs`<SidebarMenu />`);

    assert.dom().hasText('');

    // Template block usage:
    await render(hbs`
      <SidebarMenu>
        template block text
      </SidebarMenu>
    `);

    assert.dom().hasText('template block text');
  });
});
