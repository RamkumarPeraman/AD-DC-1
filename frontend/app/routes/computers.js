import Route from '@ember/routing/route';

export default class ComputerRoute extends Route {
  queryParams = {
    search: {
      refreshModel: true
    },
    sortBy: {
      refreshModel: true
    }
  }

  async model(params) {
    try {
      const search = params.search || '';
      const sortBy = params.sortBy || '';
      const response = await fetch(`http://localhost:8080/backend_war_exploded/ComputerServlet?search=${search}&sortBy=${sortBy}`);
      if (!response.ok) {
        throw new Error(`Failed to fetch computers: ${response.statusText}`);
      }
      return await response.json();
    } catch (error) {
      console.error('Error fetching computers:', error);
      return { computers: [], totalCount: 0 };
    }
  }

  setupController(controller, model) {
    super.setupController(controller, model);
    controller.computers = model.computers;
    controller.totalCount = model.totalCount;
  }
}