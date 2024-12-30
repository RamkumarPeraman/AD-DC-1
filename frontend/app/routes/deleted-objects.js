import Route from '@ember/routing/route';

export default class DeletedObjectsRoute extends Route {
  async model() {
    try {
      const response = await fetch('http://localhost:8080/backend_war_exploded/DeletedObjServlet');
      if (!response.ok) {
        throw new Error(`Failed to fetch deleted objects: ${response.statusText}`);
      }
      return await response.json();
    } catch (error) {
      console.error('Error fetching deleted objects:', error);
      return { deletedObjects: [] };
    }
  }

  setupController(controller, model) {
    super.setupController(controller, model);
    controller.deletedObjects = model.deletedObjects || [];
  }
}
