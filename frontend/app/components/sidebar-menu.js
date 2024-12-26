import Component from '@glimmer/component';
import { inject as service } from '@ember/service';

export default class SidebarMenuComponent extends Component {
  @service router;

  get currentRoute() {
    return this.router.currentRouteName;
  }
}
