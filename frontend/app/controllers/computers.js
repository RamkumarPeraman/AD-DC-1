import Controller from '@ember/controller';
import { action } from '@ember/object';
import { tracked } from '@glimmer/tracking';

export default class ComputerController extends Controller {
  @tracked computers = [];
  @tracked selectedComputer = null;
  @tracked selectedLastComputer = null;
  @tracked sortBy = '';
  @tracked searchQuery = '';
  @tracked totalCount = 0;
  @tracked name = '';
  @tracked description = '';
  @tracked location = '';
  @tracked isNewComputerPopupVisible = false;
  @tracked isComputerDetailsPopupVisible = false;
  @tracked createComputerError = '';
  @tracked deleteComputerError = '';
  @tracked isReportPopupVisible = false;
  @tracked computerCreationData = {};
  @tracked computerDetails = [];

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


  @action
  async showComputersForDay(day) {
    console.log('Fetching computers for the day:', day);
    try {
      const response = await fetch(
        `http://localhost:8080/backend_war_exploded/FetchComputerNamesForDayServlet?day=${day}`,
      );
      if (!response.ok) {
        throw new Error(
          `Failed to fetch groups for the day: ${response.statusText}`,
        );
      }
      const computerDetails = await response.json();
      this.computerDetails = computerDetails.computers;
      this.isComputerDetailsPopupVisible = true;
    } catch (error) {
      console.error('Error fetching computers for the day:', error);
    }
    console.log('hi',this.computerDetails);
  }

  @action
  closeComputerDetailsPopup() {
    this.isComputerDetailsPopupVisible = false;
    this.computerDetails = [];
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
  async showLastModDetails(computerName) {
    console.log('Fetching last modified details for computer:', computerName);
    try {
      const response = await fetch(`http://localhost:8080/backend_war_exploded/FetchLastModComp?objName=${computerName}`);
      if (!response.ok) {
        throw new Error(`Failed to fetch last modified details: ${response.statusText}`);
      }
      const data = await response.json();
      this.selectedLastComputer = {
        name: data.name || 'No Name Found',
        lastModifiedField: data.lastModifiedField || 'No Field Found',
        value: data.value || 'No Value Found',
        uSNChanged: data.uSNChanged || 'No uSNChanged Found',
        whenCreated: data.whenCreated || 'No whenCreated Found',
        whenChanged: data.whenChanged || 'No whenChanged Found',
      };
    } catch (error) {
      console.error('Error fetching last modified details:', error);
    }
    console.log('hi',this.selectedLastComputer);
  }


  @action
  closePopup() {
    this.selectedComputer = null;
    this.selectedLastComputer = null;
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
        onClick: (event, elements) => {
          console.log('Clicked on:', elements);
          console.log('Labels:', labels);
          console.log('Data:', data);
          if (elements.length > 0) {
            const index = elements[0]._index;
            console.log('Index:', index);
            const day = labels[index];
            console.log('Day:', day);
            this.showComputersForDay(day);
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
}