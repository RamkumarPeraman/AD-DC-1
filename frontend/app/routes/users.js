import Route from '@ember/routing/route';

export default class UserRoute extends Route {
  async model() {
    try {
      const response = await fetch('http://localhost:8080/backend_war_exploded/UserServlet');
      if (!response.ok) {
        throw new Error(`Failed to fetch users: ${response.statusText}`);
      }
      return await response.json();
    } catch (error) {
      console.error('Error fetching users:', error);
      return [];
    }
  }

  setupController(controller, model) {
    super.setupController(controller, model);
    controller.users = model;
  }
}
