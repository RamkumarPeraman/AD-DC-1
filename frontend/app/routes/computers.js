import Route from '@ember/routing/route';

export default class ComputerRoute extends Route {
  async model() {
    try {
      const response = await fetch('http://localhost:8080/backend_war_exploded/ComputerServlet');
      if (!response.ok) {
        throw new Error(`Failed to fetch computers: ${response.statusText}`);
      }
      return await response.json();
    } catch (error) {
      console.error('Error fetching computers:', error);
      return [];
    }
  }

  setupController(controller, model) {
    super.setupController(controller, model);
    controller.computers = model;
  }
}