import Route from '@ember/routing/route';

export default class UsersRoute extends Route {
  async model() {
    try {
      const response = await fetch(
        'http://localhost:8080/backend_war_exploded/UserServlet',
      );
      if (!response.ok) {
        throw new Error(`Failed to fetch users: ${response.statusText}`);
      }
      const users = await response.json();
      const totalCount = users.length;
      return { users, totalCount };
    } catch (error) {
      console.error('Error fetching users:', error);
      return { users: [], totalCount: 0 };
    }
  }

  setupController(controller, model) {
    super.setupController(controller, model);
    controller.users = model.users;
    controller.totalCount = model.totalCount;
  }
}
