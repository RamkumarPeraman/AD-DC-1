import Route from '@ember/routing/route';

export default class ReportRoute extends Route {
  async model() {
    try {
      const response = await fetch(
        'http://localhost:8080/backend_war_exploded/ServeDataServlet',
      );
      if (!response.ok) {
        throw new Error(`Failed to fetch report data: ${response.statusText}`);
      }
      const data = await response.json();
      return data;
    } catch (error) {
      console.error('Error fetching report data:', error);
      return [];
    }
  }

  setupController(controller, model) {
    super.setupController(controller, model);
    controller.chartData = this.processData(model);
  }

  processData(data) {
    const days = ['Jan 6', 'Jan 7', 'Jan 8'];
    const counts = { Group: [0, 0, 0], User: [0, 0, 0], Computer: [0, 0, 0] };

    data.forEach((item) => {
      const date = new Date(item.date).toLocaleDateString('en-US', {
        month: 'short',
        day: 'numeric',
      });
      const index = days.indexOf(date);
      if (index !== -1) {
        counts[item.type][index]++;
      }
    });

    return {
      labels: days,
      datasets: [
        {
          label: 'Group',
          data: counts.Group,
          backgroundColor: 'rgba(255, 99, 132, 0.2)',
          borderColor: 'rgba(255, 99, 132, 1)',
          borderWidth: 1,
        },
        {
          label: 'User',
          data: counts.User,
          backgroundColor: 'rgba(54, 162, 235, 0.2)',
          borderColor: 'rgba(54, 162, 235, 1)',
          borderWidth: 1,
        },
        {
          label: 'Computer',
          data: counts.Computer,
          backgroundColor: 'rgba(75, 192, 192, 0.2)',
          borderColor: 'rgba(75, 192, 192, 1)',
          borderWidth: 1,
        },
      ],
    };
  }
}
