import Controller from '@ember/controller';
import { action } from '@ember/object';
import { tracked } from '@glimmer/tracking';

export default class ComputerController extends Controller {
  @tracked computers = [];
  @tracked selectedComputer = null;
  @tracked sortBy = '';
  @tracked searchQuery = '';
  @tracked totalCount = 0;

  @action
  async fetchComputers() {
    try {
      const response = await fetch(`http://localhost:8080/backend_war_exploded/ComputerServlet?search=${this.searchQuery}&sortBy=${this.sortBy}`);
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
      const response = await fetch(`http://localhost:8080/backend_war_exploded/FetchComputerData?computerName=${computerName}`);
      if (!response.ok) {
        throw new Error(`Failed to fetch computer details: ${response.statusText}`);
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
}
