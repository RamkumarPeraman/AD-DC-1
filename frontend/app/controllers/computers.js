import Controller from '@ember/controller';
import { action } from '@ember/object';
import { tracked } from '@glimmer/tracking';

export default class ComputerController extends Controller {
  @tracked computers = [];
  @tracked selectedComputer = null;
  @tracked sortBy = '';
  @tracked searchQuery = '';
  @tracked totalCount = 0;
  @tracked name = '';
  @tracked description = '';
  @tracked location = '';
  @tracked isNewComputerPopupVisible = false;
  @tracked createComputerError = '';
  @tracked deleteComputerError = '';
  @tracked isReportPopupVisible = false;
  @tracked computerCreationData = {};

  constructor() {
    super(...arguments);
    this.fetchComputers();
  }

  @action
  async fetchComputers() {
    try {
      const response = await fetch(
        `http://localhost:8080/backend_war_exploded/ComputerServlet?search=${this.searchQuery}&sortBy=${this.sortBy}`,
      );
      if (!response.ok) {
        throw new Error(`Failed to fetch computers: ${response.statusText}`);
      }
      const data = await response.json();
      this.computers = data.computers;
      this.totalCount = data.totalCount;
    } catch (error) {
      console.error('Error fetching computers:', error);
      this.computers = [];
      this.totalCount = 0;
    }
  }

  @action
  async showComputerDetails(computerName) {
    try {
      const response = await fetch(
        `http://localhost:8080/backend_war_exploded/FetchComputerData?computerName=${computerName}`,
      );
      if (!response.ok) {
        throw new Error(
          `Failed to fetch computer details: ${response.statusText}`,
        );
      }
      this.selectedComputer = await response.json();
    } catch (error) {
      console.error('Error fetching computer details:', error);
      this.selectedComputer = null;
    }
  }

  @action
  closePopup() {
    this.selectedComputer = null;
  }

  @action
  updateSortBy(event) {
    this.sortBy = event.target.value;
    this.fetchComputers();
  }

  @action
  updateSearchQuery(event) {
    this.searchQuery = event.target.value;
    this.fetchComputers();
  }

  @action
  openNewComputerPopup() {
    this.isNewComputerPopupVisible = true;
  }

  @action
  closeNewComputerPopup() {
    this.isNewComputerPopupVisible = false;
    this.name = '';
    this.description = '';
    this.location = '';
    this.createComputerError = '';
  }

  @action
  updateName(event) {
    this.name = event.target.value;
  }

  @action
  updateDescription(event) {
    this.description = event.target.value;
  }

  @action
  updateLocation(event) {
    this.location = event.target.value;
  }

  @action
  async createComputer(event) {
    event.preventDefault();

    if (!this.name || !this.description || !this.location) {
      alert('All fields are required!');
      return;
    }

    try {
      const response = await fetch(
        'http://localhost:8080/backend_war_exploded/CreateComputerServlet',
        {
          method: 'POST',
          headers: {
            'Content-Type': 'application/json',
          },
          body: JSON.stringify({
            name: this.name,
            description: this.description,
            location: this.location,
          }),
        },
      );

      const result = await response.json();
      if (result.status === 'success') {
        this.fetchComputers();
        this.closeNewComputerPopup();
      } else {
        this.createComputerError =
          result.message || 'Failed to create computer!';
      }
    } catch (error) {
      console.error('Error:', error);
      this.createComputerError = 'Failed to create computer!';
    }
  }

  @action
  confirmDelete(computerName) {
    if (
      confirm(`Are you sure you want to delete the computer '${computerName}'?`)
    ) {
      this.deleteComputer(computerName);
    }
  }

  @action
  async deleteComputer(computerName) {
    try {
      const response = await fetch(
        'http://localhost:8080/backend_war_exploded/DeleteComputerServlet',
        {
          method: 'POST',
          headers: {
            'Content-Type': 'application/json',
          },
          body: JSON.stringify({
            name: computerName,
          }),
        },
      );

      const result = await response.json();
      if (result.status === 'success') {
        this.fetchComputers();
      } else {
        this.deleteComputerError =
          result.message || 'Failed to delete computer!';
      }
    } catch (error) {
      console.error('Error:', error);
      this.deleteComputerError = 'Failed to delete computer!';
    }
  }

  @action
  async fetchComputerCreationData() {
    const url = `http://localhost:8080/backend_war_exploded/ComputerCreationReportServlet`;

    try {
      const response = await fetch(url);
      if (!response.ok) {
        throw new Error(`Failed to fetch computer creation data: ${response.statusText}`);
      }
      const data = await response.json();
      this.computerCreationData = data.data;
      this.displayComputerReportChart();
    } catch (error) {
      console.error('Error fetching computer creation data:', error);
    }
  }

  @action
  openReportPopup() {
    this.isReportPopupVisible = true;
    this.fetchComputerCreationData();
  }

  @action
  closeReportPopup() {
    this.isReportPopupVisible = false;
  }

  displayComputerReportChart() {
    const ctx = document.getElementById('computerReportChart').getContext('2d');
    const labels = Object.keys(this.computerCreationData);
    const data = Object.values(this.computerCreationData).map(item => item.count);

    new Chart(ctx, {
      type: 'line',
      data: {
        labels: labels,
        datasets: [{
          label: 'Computers Created',
          data: data,
          borderColor: 'rgba(75, 192, 192, 1)',
          borderWidth: 2,
          fill: false
        }]
      },
      options: {
        scales: {
          x: {
            title: {
              display: true,
              text: 'Days'
            }
          },
          y: {
            title: {
              display: true,
              text: 'Count'
            }
          }
        },
        tooltips: {
          callbacks: {
            label: (tooltipItem) => {
              const day = labels[tooltipItem.index];
              const count = data[tooltipItem.index];
              return `Date: ${day}\nCount: ${count}`;
            }
          }
        }
      }
    });
  }
}