import Route from '@ember/routing/route';

export default class GroupRoute extends Route {
  async model() {
    const controller = this.controllerFor('group');
    await controller.fetchGroups();
  }

  setupController(controller, model) {
    super.setupController(controller, model);
  }
}
