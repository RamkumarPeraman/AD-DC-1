import Controller from '@ember/controller';
import { action } from '@ember/object';
import { tracked } from '@glimmer/tracking';
import { Chart, registerables } from 'chart.js';

Chart.register(...registerables);

export default class ReportController extends Controller {
  @tracked chartData = null;

  constructor() {
    super(...arguments);
    console.log('ReportController initialized');
  }

  @action
  async fetchChartData() {
    try {
      console.log('Fetching chart data...');
      const response = await fetch('http://localhost:8080/backend_war_exploded/ServeDataServlet');
      const data = await response.json();
      console.log('Fetched chart data:', data);
      this.chartData = this.processData(data);
      this.renderChart();
    } catch (error) {
      console.error('Error fetching chart data:', error);
    }
  }

  processData(data) {
    console.log('Processing chart data...');
    const days = ['Jan 6', 'Jan 7', 'Jan 8'];
    const counts = { Group: [0, 0, 0], User: [0, 0, 0], Computer: [0, 0, 0] };

    data.forEach(item => {
      const date = new Date(item.date).toLocaleDateString('en-US', { month: 'short', day: 'numeric' });
      const index = days.indexOf(date);
      if (index !== -1) {
        counts[item.type][index]++;
      }
    });

    return {
      labels: days,
      datasets: [
        { label: 'Group', data: counts.Group, backgroundColor: 'rgba(255, 99, 132, 0.2)', borderColor: 'rgba(255, 99, 132, 1)', borderWidth: 1 },
        { label: 'User', data: counts.User, backgroundColor: 'rgba(54, 162, 235, 0.2)', borderColor: 'rgba(54, 162, 235, 1)', borderWidth: 1 },
        { label: 'Computer', data: counts.Computer, backgroundColor: 'rgba(75, 192, 192, 0.2)', borderColor: 'rgba(75, 192, 192, 1)', borderWidth: 1 }
      ]
    };
  }

  renderChart() {
    console.log('Rendering chart...');
    const ctx = document.getElementById('myChart').getContext('2d');
    new Chart(ctx, {
      type: 'bar',
      data: this.chartData,
      options: {
        scales: { y: { beginAtZero: true } },
        plugins: {
          tooltip: {
            callbacks: {
              afterBody: function (context) {
                if (context[0].dataset.label === 'Group') {
                  return ['Group Names:', 'Group1, Group2, Group3'];
                }
              }
            }
          }
        },
        onClick: async (e, elements) => {
          if (elements.length) {
            const datasetIndex = elements[0].datasetIndex;
            const index = elements[0].index;
            const label = this.chartData.datasets[datasetIndex].label;
            if (label === 'Group') {
              const groupName = 'Group1'; // Replace with actual group name
              const response = await fetch(`http://localhost:8080/backend_war_exploded/GroupDetailsServlet?name=${groupName}`);
              const groupDetails = await response.json();
              this.displayGroupDetails(groupDetails.createdTime, groupDetails.updatedTime);
            }
          }
        }
      }
    });
  }

  displayGroupDetails(createdTime, updatedTime) {
    console.log('Displaying group details...');
    document.getElementById('groupDetails').style.display = 'block';
    document.getElementById('groupCreatedTime').innerText = `Created Time: ${createdTime}`;
    document.getElementById('groupUpdatedTime').innerText = `Updated Time: ${updatedTime}`;
  }
}