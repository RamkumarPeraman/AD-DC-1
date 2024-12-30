import Controller from '@ember/controller';
import { action } from '@ember/object';
import { tracked } from '@glimmer/tracking';

export default class UserController extends Controller {
  @tracked users = [];
  @tracked selectedUser = null;
  @tracked sortBy = '';
  @tracked searchQuery = '';
  @tracked totalCount = 0;

  @action
  async fetchUser(params = {}) {
    params.sortBy = this.sortBy;
    params.search = this.searchQuery;
    const query = new URLSearchParams(params).toString();
    const url = `http://localhost:8080/backend_war_exploded/UserServlet?${query}`;
    try {
      const response = await fetch(url);
      if (!response.ok) {
        throw new Error(`Failed to fetch users: ${response.statusText}`);
      }
      const users = await response.json();
      this.users = users;
      this.totalCount = users.length;
    } catch (error) {
      console.error('Error fetching users:', error);
      this.users = [];
      this.totalCount = 0;
    }
  }

  @action
  async showUserDetails(userName) {
    try {
      const response = await fetch(`http://localhost:8080/backend_war_exploded/FetchUserData?userName=${userName}`);
      if (!response.ok) {
        throw new Error(`Failed to fetch user details: ${response.statusText}`);
      }
      this.selectedUser = await response.json();
    } catch (error) {
      console.error('Error fetching user details:', error);
      this.selectedUser = null;
    }
  }

  @action
  closePopup() {
    this.selectedUser = null;
  }

  @action
  updateSortBy(event) {
    this.sortBy = event.target.value;
    this.fetchUser();
  }

  @action
  updateSearchQuery(event) {
    this.searchQuery = event.target.value;
    this.fetchUser();
  }
}
